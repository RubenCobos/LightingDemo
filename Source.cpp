#include "Header.h"

// INTERFACES

IDXGIFactory* Factory;
ID3D12Device* Device;
ID3D12CommandQueue* CommandQueue;
ID3D12GraphicsCommandList* CommandList;
ID3D12Fence* Fence;
IDXGISwapChain* SwapChain;
ID3D12Resource* DoubleBuffer[2];
ID3D12DescriptorHeap* RTVHeap;
ID3D12Resource* DepthBuffer;
ID3D12DescriptorHeap* DSVHeap;
ID3D12Resource* VertexBuffer;
ID3D12Resource* VertexUpload;
ID3D12Resource* IndexBuffer;
ID3D12Resource* IndexUpload;
ID3D12PipelineState* PSO;
ID3D12RootSignature* RootSignature;
ID3DBlob* VertexShaderByteCode;
ID3DBlob* PixelShaderByteCode;
ID3D12Debug* Debugger;

// DIRECT3D 12 CORE STRUCTURES

D3D12_VERTEX_BUFFER_VIEW VBV;
D3D12_INDEX_BUFFER_VIEW IBV;
D3D12_VIEWPORT ViewPortal;
D3D12_RECT ScissorRectangle;

// WINDOWS HANDLE

HWND Window;

// BACK BUFFER CLEAR COLOR

FLOAT ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

// DOUBLE BUFFER INDEX

int CurrentBackBuffer = 0;

// BUFFER VIEW SIZES

unsigned int RTVSize = 0;
unsigned int DSVSize = 0;

// COMMAND QUEUE CPU/GPU SYNCHRONIZATION

unsigned int FencePoint = 0;

// CONSTANT BUFFER ELEMENT SIZES

size_t ElementSizeOC = sizeof(ObjectConstants);
size_t ElementSizeMC = sizeof(MaterialConstants);
size_t ElementSizeLC = sizeof(LightConstants);

// FRAME QUEUE

FrameData FrameQueue[3];
unsigned int FQIndex = 2;


// SYNCHRONIZE CPU/GPU

void Synchronize(unsigned int fqindex)
{
	HANDLE event = nullptr;

	if (Fence->GetCompletedValue() < FrameQueue[fqindex].FencePoint)
	{
		event = CreateEvent(NULL, false, false, NULL);
		Fence->SetEventOnCompletion(FrameQueue[fqindex].FencePoint, event);

		if (event)
		{
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
	}
}

// UPDATE CAMERA VIEW

float xzAngle = 0.0f;
float zyAngle = 0.0f;
INT64 frequency = 0;
XMVECTOR CameraLookTo = { 0.0f, 0.0f, 1.0f, 0.0f };
XMVECTOR CameraPosition = { 0.0f, 0.0f, 0.0f, 1.0f };

static void Update()
{
	// cycle frame queue

	++FQIndex;

	if (FQIndex > 2)
	{
		FQIndex = 0;
	}

	// synchronize CPU/GPU

	Synchronize(FQIndex);

	// update Camera

	XMMATRIX horizontalRotation = XMMatrixRotationY(XMConvertToRadians(xzAngle));
	XMMATRIX verticalRotation = XMMatrixRotationX(XMConvertToRadians(zyAngle));
	XMMATRIX NetRotation = horizontalRotation * verticalRotation;
	CameraLookTo = XMVectorGetX(CameraLookTo) * NetRotation.r[0] + XMVectorGetY(CameraLookTo) * NetRotation.r[1] + XMVectorGetZ(CameraLookTo) * NetRotation.r[2] + XMVectorGetW(CameraLookTo) * NetRotation.r[3];
	CameraLookTo = XMVector3Normalize(CameraLookTo);
	CameraLookTo = XMVectorSetW(CameraLookTo, 1.0f);
	xzAngle = 0.0f;
	zyAngle = 0.0f;

	// performance timer

	INT64 CurrentTime = 0;
	static INT64 Last100thSec = 0;

	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);

	if (((CurrentTime - Last100thSec) / frequency) >= 1)
	{
		static unsigned int angleCount = 0;
		static int angleDir = 1;
		angleCount = angleCount + angleDir;
		XMVECTOR spotDir = XMLoadFloat4(&LightSources.spotAim);

		if (angleCount == 100)
		{
			spotDir = { 0.0f, 1.0f, 0.0f, 0.0f };
			angleDir = -1;
		}
		else if (angleCount == 0)
		{
			spotDir = { 0.0f, 0.0f, -1.0f, 0.0f };
			angleDir = 1;
		}
		else
		{
			XMMATRIX spotRotation = XMMatrixRotationX(angleDir * (XM_PIDIV2 / 100.0f));

			XMMATRIX spotDirMX =
			{
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f
			};

			spotDirMX.r[3] = spotDir;
			spotDirMX = spotDirMX * spotRotation;
			spotDir = spotDirMX.r[3];
			spotDir = XMVector4Normalize(spotDir);
			XMStoreFloat4(&LightSources.spotAim, spotDir);
		}

		Last100thSec = CurrentTime;
	}

	// transform Render Items

	XMVECTOR WorldUp = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMMATRIX WorldMatrix = XMMatrixIdentity();
	XMMATRIX ViewMatrix = XMMatrixLookToLH(CameraPosition, CameraLookTo, WorldUp);
	static const XMMATRIX ProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, (4.0f / 3.0f), 1.0f, 1000.0f);
	ObjectConstants Data;
	LightConstants LightData = LightSources;
	XMStoreFloat4(&LightData.camera, CameraPosition);

	for (unsigned int i = 0; i < RenderItemCount; ++i)
	{
		WorldMatrix = XMLoadFloat4x4(&RenderItems[i].WorldMatrix);
		XMStoreFloat4x4(&Data.WorldMatrix, XMMatrixTranspose(WorldMatrix));
		XMStoreFloat4x4(&Data.ViewMatrix, XMMatrixTranspose(ViewMatrix));
		XMStoreFloat4x4(&Data.ProjectionMatrix, XMMatrixTranspose(ProjectionMatrix));
		memcpy(FrameQueue[FQIndex].MappedData1 + (i * 256), &Data, ElementSizeOC);
		memcpy(FrameQueue[FQIndex].MappedData2 + (i * 256), &RenderItems[i].material, ElementSizeMC);
	}

	memcpy(FrameQueue[FQIndex].MappedData3, &LightData, ElementSizeLC);
}

// CREATE A FRAME TO PRESENT

static void Draw()
{
	// prepare to draw

	D3D12_RESOURCE_BARRIER StateTransition;
	FrameQueue[FQIndex].Commands->Reset();
	CommandList->Reset(FrameQueue[FQIndex].Commands, PSO);
	CommandList->RSSetViewports(1, &ViewPortal);
	CommandList->RSSetScissorRects(1, &ScissorRectangle);
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(DoubleBuffer[CurrentBackBuffer], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &StateTransition);
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBackBuffer, RTVSize);
	CommandList->ClearRenderTargetView(RTVHandle, ClearColor, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DSVHeap->GetCPUDescriptorHandleForHeapStart());
	CommandList->ClearDepthStencilView(DSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	CommandList->OMSetRenderTargets(1, &RTVHandle, true, &DSVHandle);
	CommandList->SetGraphicsRootSignature(RootSignature);
	CommandList->IASetVertexBuffers(0, 1, &VBV);
	CommandList->IASetIndexBuffer(&IBV);
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// issue draw calls for each render item

	D3D12_GPU_VIRTUAL_ADDRESS cbv = 0;
	cbv = FrameQueue[FQIndex].LightBuffer->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(2, cbv);

	for (size_t i = 0; i < (size_t)RenderItemCount; ++i)
	{
		cbv = FrameQueue[FQIndex].TransformBuffer->GetGPUVirtualAddress() + (i * (size_t) 256);
		CommandList->SetGraphicsRootConstantBufferView(0, cbv);
		cbv = FrameQueue[FQIndex].MaterialBuffer->GetGPUVirtualAddress() + (i * (size_t) 256);
		CommandList->SetGraphicsRootConstantBufferView(1, cbv);
		CommandList->DrawIndexedInstanced(RenderItems[i].indexCount, 1, RenderItems[i].baseIndex, RenderItems[i].baseVertex, 0);
	}

	// execute the command list and present the frame

	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(DoubleBuffer[CurrentBackBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &StateTransition);
	CommandList->Close();
	ID3D12CommandList* Commands[] = { CommandList };
	CommandQueue->ExecuteCommandLists(1, Commands);
	SwapChain->Present(0, 0);

	// signal the new command queue fence point

	++FencePoint;
	CommandQueue->Signal(Fence, FencePoint);
	FrameQueue[FQIndex].FencePoint = FencePoint;

	// swap back buffers

	if (CurrentBackBuffer)
	{
		CurrentBackBuffer = 0;
	}
	else
	{
		CurrentBackBuffer = 1;
	}
}

// UPDATE MOUSE POSITION

/* Accepting gimbal lock until more advanced math is learned. */

static void UpdateMousePosition(WPARAM MouseButton, short x, short y)
{
	static short previousX = 0;
	static short previousY = 0;

	if (MouseButton & MK_LBUTTON)
	{
		xzAngle = (1.0f / 6.0f) * (float)(x - previousX);

		if (XMVectorGetZ(CameraLookTo) > 0)
		{
			zyAngle = (1.0f / 6.0f) * (float)(y - previousY);
		}
		else // Invert rotation on the negative Z side.
		{
			zyAngle = (1.0f / 6.0f) * (float)(previousY - y);
		}
	}

	previousX = x;
	previousY = y;
}

// UPDATE CAMERA POSITION

static void UpdateCameraPosition(WPARAM wParam)
{
	switch (wParam)
	{
		case 0x57:
		{
			CameraPosition = CameraPosition + CameraLookTo;
			break;
		}
		case 0x53:
		{
			CameraPosition = CameraPosition - CameraLookTo;
			break;
		}
		case 0x44:
		{
			XMVECTOR right = XMVector3Cross({ 0.0f, 1.0f, 0.0f, 0.0f }, CameraLookTo);
			right = XMVector3Normalize(right);
			right = XMVectorSetW(right, 1.0f);
			CameraPosition = CameraPosition + right;
			break;
		}
		case 0x41:
		{
			XMVECTOR left = XMVector3Cross(CameraLookTo, { 0.0f, 1.0f, 0.0f, 0.0f });
			left = XMVector3Normalize(left);
			left = XMVectorSetW(left, 1.0f);
			CameraPosition = CameraPosition + left;
			break;
		}
		case VK_SPACE:
		{
			XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
			CameraPosition = CameraPosition + up;
			break;
		}
		case 0x43:
		{
			XMVECTOR down = { 0.0f, -1.0f, 0.0f, 0.0f };
			CameraPosition = CameraPosition + down;
			break;
		}
		default:
		{
			return;
		}
	}
}

// WINDOW PROCEDURE

static LRESULT WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_CLOSE:
		{
			DestroyWindow(window);
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			UpdateMousePosition(wparam, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			return 0;
		}
		case WM_KEYDOWN:
		{
			UpdateCameraPosition(wparam);
			return 0;
		}
		default:
		{
			return DefWindowProc(window, message, wparam, lparam);
		}
	}
}

// PROGRAM ENTRY POINT

int WinMain(_In_ HINSTANCE current, _In_opt_ HINSTANCE previous, _In_ LPSTR arguments, _In_ int show)
{
	MSG message = {};
	RECT dimensions = { 0, 0, 1024, 768 };
	WNDCLASS windowclass = {};
	windowclass.style = CS_HREDRAW | CS_VREDRAW;
	windowclass.lpfnWndProc = WindowProcedure;
	windowclass.hInstance = current;
	windowclass.lpszClassName = L"Directx";
	RegisterClass(&windowclass);
	AdjustWindowRect(&dimensions, WS_OVERLAPPEDWINDOW, false);
	Window = CreateWindow(L"Directx", L"Lighting Demo", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (dimensions.right - dimensions.left), (dimensions.bottom - dimensions.top), NULL, NULL, current, NULL);
	ShowWindow(Window, show);
	D3D12GetDebugInterface(IID_PPV_ARGS(&Debugger));
	Debugger->EnableDebugLayer();
	BuildWorldGeometry();
	CalculateSurfaceNormals();
	Initialize();

	while (message.message != WM_QUIT)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			Update();
			Draw();
		}
	}

	Synchronize(FQIndex);
	RTVHeap->Release();
	DSVHeap->Release();
	DoubleBuffer[0]->Release();
	DoubleBuffer[1]->Release();
	SwapChain->Release();
	DepthBuffer->Release();
	VertexBuffer->Release();
	IndexBuffer->Release();
	FrameQueue[0].TransformBuffer->Unmap(0, nullptr);
	FrameQueue[1].TransformBuffer->Unmap(0, nullptr);
	FrameQueue[2].TransformBuffer->Unmap(0, nullptr);
	FrameQueue[0].TransformBuffer->Release();
	FrameQueue[1].TransformBuffer->Release();
	FrameQueue[2].TransformBuffer->Release();
	FrameQueue[0].MaterialBuffer->Unmap(0, nullptr);
	FrameQueue[1].MaterialBuffer->Unmap(0, nullptr);
	FrameQueue[2].MaterialBuffer->Unmap(0, nullptr);
	FrameQueue[0].MaterialBuffer->Release();
	FrameQueue[1].MaterialBuffer->Release();
	FrameQueue[2].MaterialBuffer->Release();
	FrameQueue[0].LightBuffer->Unmap(0, nullptr);
	FrameQueue[1].LightBuffer->Unmap(0, nullptr);
	FrameQueue[2].LightBuffer->Unmap(0, nullptr);
	FrameQueue[0].LightBuffer->Release();
	FrameQueue[1].LightBuffer->Release();
	FrameQueue[2].LightBuffer->Release();
	RootSignature->Release();
	PSO->Release();
	FrameQueue[0].Commands->Release();
	FrameQueue[1].Commands->Release();
	FrameQueue[2].Commands->Release();
	CommandList->Release();
	CommandQueue->Release();
	Fence->Release();
	Device->Release();
	Factory->Release();
	Debugger->Release();
	return (int)message.wParam;
}

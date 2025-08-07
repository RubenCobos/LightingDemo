#include <Windows.h>
#include <Windowsx.h>
#include <dxgi.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <fstream>

using namespace DirectX;
using namespace std;

// INTERFACES

extern IDXGIFactory* Factory;
extern ID3D12Device* Device;
extern ID3D12CommandQueue* CommandQueue;
extern ID3D12GraphicsCommandList* CommandList;
extern ID3D12Fence* Fence;
extern IDXGISwapChain* SwapChain;
extern ID3D12Resource* DoubleBuffer[2];
extern ID3D12DescriptorHeap* RTVHeap;
extern ID3D12Resource* DepthBuffer;
extern ID3D12DescriptorHeap* DSVHeap;
extern ID3D12Resource* VertexBuffer;
extern ID3D12Resource* VertexUpload;
extern ID3D12Resource* IndexBuffer;
extern ID3D12Resource* IndexUpload;
extern ID3D12PipelineState* PSO;
extern ID3D12RootSignature* RootSignature;
extern ID3DBlob* VertexShaderByteCode;
extern ID3DBlob* PixelShaderByteCode;
extern ID3D12Debug* Debugger;

// VERTICES

struct Vertex
{
	XMFLOAT4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 normal = { 0.0f, 0.0f, 0.0f, 0.0f };
};

// GPU TRANSFORMATION DATA

struct ObjectConstants
{
	XMFLOAT4X4 WorldMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	XMFLOAT4X4 ViewMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	XMFLOAT4X4 ProjectionMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
};

// GPU MATERIAL DATA

struct MaterialConstants
{
	XMFLOAT4 DiffuseAlbedo = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 SpecProperty = { 0.0f, 0.0f, 0.0f, 1.0f };

	int roughness = 0;
};

// GPU LIGHT DATA

struct LightConstants
{
	XMFLOAT4 directional = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT4 point = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 spot = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 spotAim = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 camera = { 0.0f, 0.0f, 0.0f, 1.0f };
};

// FRAME DATA

struct FrameData
{
	ID3D12CommandAllocator* Commands = nullptr;

	ID3D12Resource* TransformBuffer = nullptr;
	ID3D12Resource* MaterialBuffer = nullptr;
	ID3D12Resource* LightBuffer = nullptr;

	BYTE* MappedData1 = nullptr;
	BYTE* MappedData2 = nullptr;
	BYTE* MappedData3 = nullptr;

	unsigned int FencePoint = 0;
};

// CPU GEOMETRY DATA

struct GeometryObject
{
	MaterialConstants material;

	XMFLOAT4X4 WorldMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int baseVertex = 0;
	unsigned int vertexCount = 0;
	unsigned int baseIndex = 0;
	unsigned int indexCount = 0;
	unsigned int instance = 0;
};

// GLOBAL STRUCTURES

extern D3D12_VERTEX_BUFFER_VIEW VBV;
extern D3D12_INDEX_BUFFER_VIEW IBV;
extern D3D12_VIEWPORT ViewPortal;
extern D3D12_RECT ScissorRectangle;

// GLOBAL VARIABLES

extern HWND Window;
extern FrameData FrameQueue[3];
extern LightConstants LightSources;
extern vector<Vertex> VertexArray;
extern vector<UINT16> IndexArray;
extern vector<GeometryObject> RenderItems;
extern XMVECTOR spotAimStart;
extern INT64 frequency;
extern int CurrentBackBuffer;
extern unsigned int RenderItemCount;
extern unsigned int FencePoint;
extern unsigned int RTVSize;
extern unsigned int DSVSize;

// GLOABL FUNCTIONS

void Initialize();
void Synchronize(unsigned int);
void BuildWorldGeometry();
void CalculateSurfaceNormals();
void ShowNormals();

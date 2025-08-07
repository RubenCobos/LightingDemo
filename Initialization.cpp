#include "Header.h"

// INITIALIZATION FUNCTIONS

static void CreateFactory()
{
	CreateDXGIFactory(IID_PPV_ARGS(&Factory));
}

static void CreateDevice()
{
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device));
}

static void CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC Descriptor =
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};

	Device->CreateCommandQueue(&Descriptor, IID_PPV_ARGS(&CommandQueue));
	Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameQueue[0].Commands));
	Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameQueue[1].Commands));
	Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameQueue[2].Commands));
	Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, FrameQueue[0].Commands, nullptr, IID_PPV_ARGS(&CommandList));
	Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
}

static void CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC Descriptor =
	{
		{
			1024,
			768,
			{ 60, 1 },
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
			DXGI_MODE_SCALING_UNSPECIFIED
		},
		{ 1, 0 },
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		2,
		Window,
		true,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	Factory->CreateSwapChain(CommandQueue, &Descriptor, &SwapChain);
	SwapChain->GetBuffer(0, IID_PPV_ARGS(&DoubleBuffer[0]));
	SwapChain->GetBuffer(1, IID_PPV_ARGS(&DoubleBuffer[1]));
}

static void CreateRenderTargetView()
{
	D3D12_DESCRIPTOR_HEAP_DESC Descriptor =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		2,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};

	D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle;
	Device->CreateDescriptorHeap(&Descriptor, IID_PPV_ARGS(&RTVHeap));
	RTVSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	HeapHandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();
	Device->CreateRenderTargetView(DoubleBuffer[0], nullptr, HeapHandle);
	HeapHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(HeapHandle, 1, RTVSize);
	Device->CreateRenderTargetView(DoubleBuffer[1], nullptr, HeapHandle);
}

static void CreateDepthBuffer()
{
	D3D12_RESOURCE_BARRIER StateTransition;

	D3D12_RESOURCE_DESC ResourceDescriptor =
	{
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		1024,
		768,
		1,
		1,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		{ 1, 0 },
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	};

	D3D12_CLEAR_VALUE OptimizedClear =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		{ 1.0f, 0 }
	};

	D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, &OptimizedClear, IID_PPV_ARGS(&DepthBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(DepthBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	CommandList->ResourceBarrier(1, &StateTransition);

	D3D12_DESCRIPTOR_HEAP_DESC HeapDescriptor =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};

	Device->CreateDescriptorHeap(&HeapDescriptor, IID_PPV_ARGS(&DSVHeap));
	DSVSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
	Device->CreateDepthStencilView(DepthBuffer, nullptr, HeapHandle);
}

static void CreateVertexBuffer()
{
	D3D12_RESOURCE_BARRIER StateTransition;
	D3D12_RESOURCE_DESC Descriptor = CD3DX12_RESOURCE_DESC::Buffer(VertexArray.size() * sizeof(Vertex));
	D3D12_HEAP_PROPERTIES Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	Device->CreateCommittedResource(&Properties, D3D12_HEAP_FLAG_NONE, &Descriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&VertexBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	CommandList->ResourceBarrier(1, &StateTransition);
	Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	Device->CreateCommittedResource(&Properties, D3D12_HEAP_FLAG_NONE, &Descriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&VertexUpload));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(VertexUpload, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);

	D3D12_SUBRESOURCE_DATA UploadData =
	{
		VertexArray.data(),
		(LONG_PTR) (VertexArray.size() * sizeof(Vertex)),
		(LONG_PTR) (VertexArray.size() * sizeof(Vertex))
	};

	UpdateSubresources<1>(CommandList, VertexBuffer, VertexUpload, 0, 0, 1, &UploadData);
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	VBV.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
	VBV.SizeInBytes = (((UINT) VertexArray.size()) * sizeof(Vertex));
	VBV.StrideInBytes = (sizeof(Vertex));
}

static void CreateIndexBuffer()
{
	D3D12_RESOURCE_BARRIER StateTransition;
	D3D12_RESOURCE_DESC Descriptor = CD3DX12_RESOURCE_DESC::Buffer(IndexArray.size() * sizeof(UINT16));
	D3D12_HEAP_PROPERTIES Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	Device->CreateCommittedResource(&Properties, D3D12_HEAP_FLAG_NONE, &Descriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&IndexBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	CommandList->ResourceBarrier(1, &StateTransition);
	Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	Device->CreateCommittedResource(&Properties, D3D12_HEAP_FLAG_NONE, &Descriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&IndexUpload));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(IndexUpload, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);

	D3D12_SUBRESOURCE_DATA UploadData =
	{
		IndexArray.data(),
		(LONG_PTR) (IndexArray.size() * sizeof(UINT16)),
		(LONG_PTR) (IndexArray.size() * sizeof(UINT16))
	};

	UpdateSubresources<1>(CommandList, IndexBuffer, IndexUpload, 0, 0, 1, &UploadData);
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	IBV.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	IBV.Format = DXGI_FORMAT_R16_UINT;
	IBV.SizeInBytes = (((UINT) IndexArray.size()) * sizeof(UINT16));
}

static void CreateConstantBuffer()
{
	D3D12_RESOURCE_BARRIER StateTransition;
	D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	// FRAME 1 DATA
	D3D12_RESOURCE_DESC ResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer((UINT64)(256 * RenderItemCount));

	// transformation data
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[0].TransformBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[0].TransformBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[0].TransformBuffer->Map(0, nullptr, (void**) &FrameQueue[0].MappedData1);
	ZeroMemory(FrameQueue[0].MappedData1, RenderItemCount * 256);

	// material data
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[0].MaterialBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[0].MaterialBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[0].MaterialBuffer->Map(0, nullptr, (void**)&FrameQueue[0].MappedData2);
	ZeroMemory(FrameQueue[0].MappedData2, RenderItemCount * 256);

	// light data
	ResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer((UINT64) 256);
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[0].LightBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[0].LightBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[0].LightBuffer->Map(0, nullptr, (void**)&FrameQueue[0].MappedData3);
	ZeroMemory(FrameQueue[0].MappedData3, 256);

	// FRAME 2 DATA
	ResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer((UINT64)(256 * RenderItemCount));

	// transformation data
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[1].TransformBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[1].TransformBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[1].TransformBuffer->Map(0, nullptr, (void**)&FrameQueue[1].MappedData1);
	ZeroMemory(FrameQueue[1].MappedData1, RenderItemCount * 256);

	// material data
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[1].MaterialBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[1].MaterialBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[1].MaterialBuffer->Map(0, nullptr, (void**)&FrameQueue[1].MappedData2);
	ZeroMemory(FrameQueue[1].MappedData2, RenderItemCount * 256);

	// light data
	ResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer((UINT64) 256);
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[1].LightBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[1].LightBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[1].LightBuffer->Map(0, nullptr, (void**)&FrameQueue[1].MappedData3);
	ZeroMemory(FrameQueue[1].MappedData3, 256);

	// FRAME 3 DATA
	ResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer((UINT64)(256 * RenderItemCount));

	// transformation data
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[2].TransformBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[2].TransformBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[2].TransformBuffer->Map(0, nullptr, (void**)&FrameQueue[2].MappedData1);
	ZeroMemory(FrameQueue[2].MappedData1, RenderItemCount * 256);

	// material data
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[2].MaterialBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[2].MaterialBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[2].MaterialBuffer->Map(0, nullptr, (void**)&FrameQueue[2].MappedData2);
	ZeroMemory(FrameQueue[2].MappedData2, RenderItemCount * 256);

	// light data
	ResourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer((UINT64) 256);
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescriptor, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&FrameQueue[2].LightBuffer));
	StateTransition = CD3DX12_RESOURCE_BARRIER::Transition(FrameQueue[2].LightBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	CommandList->ResourceBarrier(1, &StateTransition);
	FrameQueue[2].LightBuffer->Map(0, nullptr, (void**)&FrameQueue[2].MappedData3);
	ZeroMemory(FrameQueue[2].MappedData3, 256);
}

static void CreateRootSignature()
{
	CD3DX12_ROOT_PARAMETER RootParameters[3] =
	{
		CD3DX12_ROOT_PARAMETER()
	};

	RootParameters[0].InitAsConstantBufferView(0);
	RootParameters[1].InitAsConstantBufferView(1);
	RootParameters[2].InitAsConstantBufferView(2);
	CD3DX12_ROOT_SIGNATURE_DESC Descriptor(3, RootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ID3DBlob* SerializedRootSig;
	ID3DBlob* RootSigError;
	D3D12SerializeRootSignature(&Descriptor, D3D_ROOT_SIGNATURE_VERSION_1, &SerializedRootSig, &RootSigError);
	Device->CreateRootSignature(0, SerializedRootSig->GetBufferPointer(), SerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&RootSignature));
	SerializedRootSig->Release();

	if (RootSigError != nullptr)
	{
		RootSigError->Release();
	}
}

static void CreateShaderPrograms()
{
	long long size = 0;
	fstream InputFile;
	InputFile.open("vs.cso", ios::in | ios::binary);
	InputFile.seekg(0, ios::end);
	size = InputFile.tellg();
	D3DCreateBlob(size, &VertexShaderByteCode);
	InputFile.seekg(0, ios::beg);
	InputFile.read((char*)VertexShaderByteCode->GetBufferPointer(), size);
	InputFile.close();
	InputFile.open("ps.cso", ios::in | ios::binary);
	InputFile.seekg(0, ios::end);
	size = InputFile.tellg();
	D3DCreateBlob(size, &PixelShaderByteCode);
	InputFile.seekg(0, ios::beg);
	InputFile.read((char*)PixelShaderByteCode->GetBufferPointer(), size);
	InputFile.close();
}

static void CreatePipelineStateObject()
{
	D3D12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	D3D12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	D3D12_DEPTH_STENCIL_DESC DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	D3D12_INPUT_ELEMENT_DESC VertexComponents[2] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexInputLayout = { VertexComponents, 2 };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Descriptor;
	ZeroMemory(&Descriptor, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	Descriptor.pRootSignature = RootSignature;
	Descriptor.VS.pShaderBytecode = VertexShaderByteCode->GetBufferPointer();
	Descriptor.VS.BytecodeLength = VertexShaderByteCode->GetBufferSize();
	Descriptor.PS.pShaderBytecode = PixelShaderByteCode->GetBufferPointer();
	Descriptor.PS.BytecodeLength = PixelShaderByteCode->GetBufferSize();
	Descriptor.BlendState = BlendState;
	Descriptor.SampleMask = UINT_MAX;
	Descriptor.RasterizerState = RasterizerState;
	Descriptor.DepthStencilState = DepthStencilState;
	Descriptor.InputLayout = VertexInputLayout;
	Descriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Descriptor.NumRenderTargets = 1;
	Descriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	Descriptor.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	Descriptor.SampleDesc.Count = 1;
	Descriptor.SampleDesc.Quality = 0;
	Device->CreateGraphicsPipelineState(&Descriptor, IID_PPV_ARGS(&PSO));
}

void Initialize()
{
	CreateFactory();
	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain();
	CreateRenderTargetView();
	CreateDepthBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateConstantBuffer();
	CreateRootSignature();
	CreateShaderPrograms();
	CreatePipelineStateObject();
	ViewPortal.TopLeftX = 0.0f;
	ViewPortal.TopLeftY = 0.0f;
	ViewPortal.Width = (float) 1024;
	ViewPortal.Height = (float) 768;
	ViewPortal.MinDepth = 0.0f;
	ViewPortal.MaxDepth = 1.0f;
	ScissorRectangle.left = 0;
	ScissorRectangle.top = 0;
	ScissorRectangle.right = 1024;
	ScissorRectangle.bottom = 768;
	CommandList->Close();
	ID3D12CommandList* Commands[] = { CommandList };
	CommandQueue->ExecuteCommandLists(1, Commands);
	++FencePoint;
	CommandQueue->Signal(Fence, FencePoint);
	FrameQueue[0].FencePoint = FencePoint;
	Synchronize(0);
	VertexUpload->Release();
	IndexUpload->Release();
	VertexShaderByteCode->Release();
	PixelShaderByteCode->Release();
	QueryPerformanceFrequency((LARGE_INTEGER*) & frequency);
	frequency = frequency / 100;
}

#include "UIRenderer.h"

void UIRenderer::ConvertValues()
{
	auto worldPos = attachFrom->GetWorldPosition();

	float posX = DirectX::XMVectorGetX(worldPos);
	float posY = DirectX::XMVectorGetY(worldPos);

	auto position_mat = XMMatrixTranslation(posX, posY, 0);

	auto worldZ = DirectX::XMVectorGetZ(attachFrom->GetWorldRotation());
	auto localZ = DirectX::XMVectorGetZ(attachFrom->GetLocalRotation());

	worldZ = DirectX::XMConvertToRadians(worldZ);
	localZ = DirectX::XMConvertToRadians(localZ);

	auto rotation_mat = DirectX::XMMatrixRotationZ(worldZ + localZ);

	auto scale = attachFrom->GetScale();

	float scaleX = DirectX::XMVectorGetX(scale);
	float scaleY = DirectX::XMVectorGetY(scale);

	auto scale_mat = XMMatrixScaling(scaleX, scaleY, 1);

	trans.world = scale_mat * rotation_mat * position_mat;

	float colR = DirectX::XMVectorGetX(color);
	float colG = DirectX::XMVectorGetY(color);
	float colB = DirectX::XMVectorGetZ(color);
	float colA = DirectX::XMVectorGetW(color);

	auto color_normalized = XMFLOAT4(colR, colG, colB, colA);

	trans.color = color_normalized;
}

bool UIRenderer::MapCB()
{
	ConvertValues();

	trans.view = XMMatrixLookAtLH(Utility::zero3D, Utility::forward, DirectX::XMVectorSet(0, 1, 0, 0));

	trans.projection = DirectX::XMMatrixOrthographicLH(Window::width, Window::height, 1 , 1000);
	
	void* ptr[2] = { nullptr , nullptr };

	for (int i = 0; i < 2; i++)
	{
		ptr[i] = nullptr;

		hr = constantBuffer[i]->Map(0, nullptr, &ptr[i]);

		if (FAILED(hr))
		{
			std::cerr << "座標行列が収納されたCBのマッピング失敗" << std::endl;

			return false;
		}

		memcpy(ptr[i], &trans, sizeof(Transform));


		constantBuffer[i]->Unmap(0, nullptr);
	}

	return true;
}

UIRenderer::UIRenderer(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
	bool isSuccess = true;

	isSuccess &= InitDescHeap();

	isSuccess &= InitCBVResource();

	isSuccess &= LinkCBVResourceToDescHeap();

	isSuccess &= MapCB();

	isSuccess &= InitVBVResource();

	isSuccess &= MapVB();

	isSuccess &= InitIBVResource();

	isSuccess &= MapIB();

	isSuccess &= CreateRootSignature();

	isSuccess &= CreateRenderPipeline();

	isSuccess &= LoadTexture("Textures/error.dds");

	if (!isSuccess)
	{
		std::cerr << "UIRendererの初期化失敗" << std::endl;
	}
}

UIRenderer::~UIRenderer()
{
}

void UIRenderer::AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex)
{
	MapCB();

	cmdList->SetGraphicsRootSignature(rootSig.Get());

	cmdList->SetDescriptorHeaps(1, heap.GetAddressOf());

	cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);

	cmdList->SetPipelineState(pipeline.Get());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->SetGraphicsRootDescriptorTable(1, texView.gpu_handle);

	cmdList->IASetVertexBuffers(0, 1, &vbView);

	cmdList->IASetIndexBuffer(&ibView);

	cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}

bool UIRenderer::InitDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	desc.NodeMask = 0;

	desc.NumDescriptors = DXRenderSystem::frameBuffer + 2;

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "デスクリプタヒープ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool UIRenderer::InitCBVResource()
{
	for (int i = 0; i < DXRenderSystem::frameBuffer; i++)
	{
		D3D12_HEAP_PROPERTIES prop = {};

		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

		prop.Type = D3D12_HEAP_TYPE_UPLOAD;

		prop.CreationNodeMask = 1;

		prop.VisibleNodeMask = 1;

		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc = {  };

		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

		desc.Alignment = 0;

		desc.Height = 1;

		desc.Width = sizeof(Transform);

		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		desc.SampleDesc.Count = 1;

		desc.SampleDesc.Quality = 0;

		desc.DepthOrArraySize = 1;

		desc.MipLevels = 1;

		desc.Format = DXGI_FORMAT_UNKNOWN;

		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(constantBuffer[i].GetAddressOf())

		);

		if (FAILED(hr))
		{
			std::cerr << "CBVのリソース作成失敗" << std::endl;

			return false;
		}
	}

	return true;
}

bool UIRenderer::LinkCBVResourceToDescHeap()
{
	auto increament = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto heapPtr = heap->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < DXRenderSystem::frameBuffer; i++)
	{
		cbView[i].BufferLocation = constantBuffer[i]->GetGPUVirtualAddress();

		cbView[i].SizeInBytes = sizeof(Transform);

		heapPtr.ptr += increament;

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&cbView[i], heapPtr);
	}

	return true;
}

bool UIRenderer::InitVBVResource()
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(VertexData) * vertexDatas.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadVertexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "VBVのアップロードリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool UIRenderer::MapVB()
{
	void* ptr = nullptr;

	hr = uploadVertexBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "頂点データのマッピング失敗" << std::endl;

		return false;
	}
	memcpy(ptr, vertexDatas.data(), sizeof(VertexData) * vertexDatas.size());

	uploadVertexBuffer->Unmap(0, nullptr);

	return VBUploadToDefultHeap();
}

bool UIRenderer::VBUploadToDefultHeap()
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(VertexData) * vertexDatas.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(vertexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "VBVのリソース作成失敗" << std::endl;

		return false;
	}

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = vertexDatas.data();
	subresourceData.RowPitch = sizeof(VertexData) * vertexDatas.size();
	subresourceData.SlicePitch = subresourceData.RowPitch;

	UpdateSubresources(DXRenderSystem::GetInstance().GetCommandList().Get(), vertexBuffer.Get(), uploadVertexBuffer.Get(), 0, 0, 1, &subresourceData);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
	);
	DXRenderSystem::GetInstance().GetCommandList()->ResourceBarrier(1, &barrier);

	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(VertexData) * vertexDatas.size();
	vbView.StrideInBytes = sizeof(VertexData);

	return true;
}

bool UIRenderer::InitIBVResource()
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(int) * indices.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadIndexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "IBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool UIRenderer::MapIB()
{
	void* ptr = nullptr;

	hr = uploadIndexBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "インデックスデータのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, indices.data(), sizeof(int) * indices.size());

	uploadIndexBuffer->Unmap(0, nullptr);

	return IBUploadToDefultHeap();
}

bool UIRenderer::IBUploadToDefultHeap()
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(int) * indices.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(indexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "IBVのリソース作成失敗" << std::endl;

		return false;
	}

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = indices.data();
	subresourceData.RowPitch = sizeof(int) * indices.size();
	subresourceData.SlicePitch = subresourceData.RowPitch;

	UpdateSubresources(DXRenderSystem::GetInstance().GetCommandList().Get(), indexBuffer.Get(), uploadIndexBuffer.Get(), 0, 0, 1, &subresourceData);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER
	);
	DXRenderSystem::GetInstance().GetCommandList()->ResourceBarrier(1, &barrier);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.SizeInBytes = sizeof(int) * indices.size();;
	ibView.Format = DXGI_FORMAT_R32_UINT;

	return true;
}

bool UIRenderer::CreateRootSignature()
{
	D3D12_ROOT_PARAMETER param[2] = {};

	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[0].Descriptor.ShaderRegister = 0;

	param[0].Descriptor.RegisterSpace = 0;

	D3D12_DESCRIPTOR_RANGE range = {};

	range.NumDescriptors = 1;

	range.BaseShaderRegister = 0;

	range.RegisterSpace = 0;

	range.OffsetInDescriptorsFromTableStart = 0;

	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	param[1].DescriptorTable.NumDescriptorRanges = 1;

	param[1].DescriptorTable.pDescriptorRanges = &range;

	D3D12_STATIC_SAMPLER_DESC desc_sampler = {};

	desc_sampler.Filter = D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;

	desc_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desc_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desc_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desc_sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	desc_sampler.MaxAnisotropy = 1;
	desc_sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	desc_sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	desc_sampler.MaxLOD = D3D12_FLOAT32_MAX;
	desc_sampler.MinLOD = -D3D12_FLOAT32_MAX;
	desc_sampler.ShaderRegister = 0;
	desc_sampler.RegisterSpace = 0;
	desc_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	desc.NumParameters = sizeof(param) / sizeof(D3D12_ROOT_PARAMETER);

	desc.NumStaticSamplers = 1;

	desc.pParameters = param;

	desc.pStaticSamplers = &desc_sampler;

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error_blob;

	hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddressOf(), error_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ルートシグネチャシリアライズ化失敗" << std::endl;

		return false;
	}

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "ルートシグネチャ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool UIRenderer::CreateRenderPipeline()
{
	D3D12_INPUT_ELEMENT_DESC desc_element[2] = {
	{"POSITION" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"TEXCOORD" , 0 , DXGI_FORMAT_R32G32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	};

	D3D12_RASTERIZER_DESC desc_ras = {};

	desc_ras.CullMode = D3D12_CULL_MODE_NONE;

	desc_ras.FillMode = D3D12_FILL_MODE_SOLID;

	desc_ras.FrontCounterClockwise = false;

	desc_ras.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;

	desc_ras.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;

	desc_ras.DepthClipEnable = false;

	desc_ras.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

	desc_ras.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	desc_ras.MultisampleEnable = false;

	desc_ras.AntialiasedLineEnable = false;

	desc_ras.ForcedSampleCount = 0;

	D3D12_RENDER_TARGET_BLEND_DESC desc_rt_blend = {};

	desc_rt_blend.BlendEnable = true;
	desc_rt_blend.BlendOp = D3D12_BLEND_OP_ADD;
	desc_rt_blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	desc_rt_blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	desc_rt_blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	desc_rt_blend.LogicOp = D3D12_LOGIC_OP_NOOP;
	desc_rt_blend.SrcBlendAlpha = D3D12_BLEND_ONE;
	desc_rt_blend.DestBlendAlpha = D3D12_BLEND_ZERO;
	desc_rt_blend.LogicOpEnable = false;
	desc_rt_blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC desc_blend = {};

	desc_blend.AlphaToCoverageEnable = false;

	desc_blend.IndependentBlendEnable = false;

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
	{
		desc_blend.RenderTarget[i] = desc_rt_blend;
	}


	ComPtr<ID3DBlob> vs_blob;
	ComPtr<ID3DBlob> ps_blob;

	hr = D3DReadFileToBlob(L"UIVertexShader.cso", vs_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "頂点シェーダー読み込みエラー" << std::endl;

		return false;
	}

	hr = D3DReadFileToBlob(L"UIPixelShader.cso", ps_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ピクセルシェーダー読み込みエラー" << std::endl;

		return false;
	}

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = {};
	dsDesc.BackFace = {};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

	desc_pipe.InputLayout = { desc_element , _countof(desc_element) };

	desc_pipe.pRootSignature = rootSig.Get();

	desc_pipe.VS = { vs_blob->GetBufferPointer() , vs_blob->GetBufferSize() };

	desc_pipe.PS = { ps_blob->GetBufferPointer() , ps_blob->GetBufferSize() };

	desc_pipe.RasterizerState = desc_ras;

	desc_pipe.BlendState = desc_blend;

	desc_pipe.DepthStencilState = dsDesc;

	desc_pipe.SampleMask = UINT_MAX;

	desc_pipe.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	desc_pipe.NumRenderTargets = 1;

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
	{
		desc_pipe.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}

	desc_pipe.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	desc_pipe.SampleDesc.Count = 1;

	desc_pipe.SampleDesc.Quality = 0;


	hr = DXRenderSystem::GetInstance().GetDevice()->CreateGraphicsPipelineState(&desc_pipe, IID_PPV_ARGS(pipeline.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "パイプラインの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool UIRenderer::LoadTexture(const std::string fileName)
{

	int len = MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), -1, NULL, 0);
	std::wstring ws(len, L'\0');

	MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), -1, &ws[0], len);

	ws.pop_back();

	DirectX::ResourceUploadBatch batch(DXRenderSystem::GetInstance().GetDevice().Get());

	batch.Begin();

	hr = DirectX::CreateDDSTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, ws.c_str(), texView.resource.GetAddressOf(), true);

	int texHeight = texView.resource->GetDesc().Height / 2;
	int texWidth = texView.resource->GetDesc().Width / 2;

	vertexDatas[0].vertex = DirectX::XMFLOAT3(-texWidth , texHeight , 0);
	vertexDatas[1].vertex = DirectX::XMFLOAT3(texWidth , texHeight , 0);
	vertexDatas[2].vertex = DirectX::XMFLOAT3(texWidth , -texHeight , 0);
	vertexDatas[3].vertex = DirectX::XMFLOAT3(-texWidth , -texHeight , 0);

	MapVB();

	std::wcout << L"path = " << ws << std::endl;

	if (FAILED(hr))
	{
		std::cerr << "ddsファイル読み込みエラー" << std::endl;

		hr = DirectX::CreateDDSTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, L"Textures/error.dds", texView.resource.GetAddressOf(), true);
	}

	auto load = batch.End(DXRenderSystem::GetInstance().GetCommandQueue().Get());

	load.wait();

	UINT descriptorSize = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT ptrNum = DXRenderSystem::frameBuffer + 1;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += ptrNum * descriptorSize;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += ptrNum * descriptorSize;

	texView.cpu_handle = cpuHandle;
	texView.gpu_handle = gpuHandle;

	auto texture_desc = texView.resource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	desc.Format = texture_desc.Format;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.Texture2D.MipLevels = texture_desc.MipLevels;
	desc.Texture2D.PlaneSlice = 0;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(texView.resource.Get(), &desc, cpuHandle);

	return true;
}

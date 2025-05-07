#include "Renderer.h"
#include "Utility.h"
#include <filesystem>

void Renderer::ConvertValues()
{
	auto worldPos = attachFrom->GetWorldPosition();

	float posX = DirectX::XMVectorGetX(worldPos);
	float posY = DirectX::XMVectorGetY(worldPos);
	float posZ = DirectX::XMVectorGetZ(worldPos);

	auto position_mat = XMMatrixTranslation(posX, posY, posZ);

	auto mat1 = DirectX::XMMatrixRotationQuaternion(attachFrom->GetWorldQuaternion());
	auto mat2 = DirectX::XMMatrixRotationQuaternion(attachFrom->GetLocalQuaternion());

	auto rotation_mat = mat1 * mat2;

	auto scale = attachFrom->GetScale();

	float scaleX = DirectX::XMVectorGetX(scale);
	float scaleY = DirectX::XMVectorGetY(scale);
	float scaleZ = DirectX::XMVectorGetZ(scale);

	auto scale_mat = XMMatrixScaling(scaleX, scaleY, scaleZ);

	trans.world = scale_mat * rotation_mat * position_mat;

	float colR = DirectX::XMVectorGetX(color);
	float colG = DirectX::XMVectorGetY(color);
	float colB = DirectX::XMVectorGetZ(color);
	float colA = DirectX::XMVectorGetW(color);

	auto color_normalized = XMFLOAT4(colR, colG, colB , colA);

	trans.color = color_normalized;
}

bool Renderer::MapCB()
{
	ConvertValues();

	trans.view = XMMatrixLookAtLH(Camera::GetInstance().GetPosition(), Camera::GetInstance().GetTargetPos(), DirectX::XMVectorSet(0,1,0,0));

	trans.projection = XMMatrixPerspectiveFovLH(Camera::GetInstance().fov, Camera::GetInstance().aspect, 1, 1000);

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

Renderer::Renderer(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
	finalBoneMats.resize(200);

	bool isSuccess = true;

	isSuccess &= InitDescHeap();

	isSuccess &= InitCBVResource();

	isSuccess &= LinkCBVResourceToDescHeap();

	isSuccess &= MapCB();

	isSuccess &= InitBoneResource();

	isSuccess &= MapBB();

	isSuccess &= InitSRVForBoneBuffer();

	for (int i =  0 ; i < meshDatas.size() ; i++)
	{
		auto& mesh = meshDatas[i];

		isSuccess &= InitVBVResource(mesh);

		isSuccess &= MapVB(mesh);

		isSuccess &= InitIBVResource(mesh);

		isSuccess &= MapIB(mesh);

		isSuccess &= LoadTexture("Textures/error.dds" , mesh );
	}

	isSuccess &= CreateRootSignature();

	isSuccess &= CreateRenderPipeline();

	if (!isSuccess)
	{
		std::cerr << "レンダーコンポネントの初期化失敗" << std::endl;
	}
}

Renderer::~Renderer()
{
}

void Renderer::AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex)
{
	MapCB();

	cmdList->SetGraphicsRootSignature(rootSig.Get());

	cmdList->SetDescriptorHeaps(1, heap.GetAddressOf());

	cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);


	cmdList->SetPipelineState(pipeline.Get());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& mesh : meshDatas)
	{
		cmdList->SetGraphicsRootDescriptorTable(1, mesh.texView.gpu_handle);

		cmdList->SetGraphicsRootShaderResourceView(2, boneBuffer->GetGPUVirtualAddress());
		cmdList->IASetVertexBuffers(0, 1, &mesh.vbView);

		cmdList->IASetIndexBuffer(&mesh.ibView);

		cmdList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}

}

UINT Renderer::GetHandlingNum()
{
	auto num = numHandling;

	std::cout << num << std::endl;

	numHandling++;

	return num;
}

bool Renderer::InitDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	desc.NodeMask = 0;

	desc.NumDescriptors = DXRenderSystem::frameBuffer + meshDatas.size() + 100;

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "デスクリプタヒープ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::InitCBVResource()
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

bool Renderer::LinkCBVResourceToDescHeap()
{
	auto increament = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0; i < DXRenderSystem::frameBuffer; i++)
	{
		auto heapPtr = heap->GetCPUDescriptorHandleForHeapStart();

		cbView[i].BufferLocation = constantBuffer[i]->GetGPUVirtualAddress();

		cbView[i].SizeInBytes = sizeof(Transform);

		heapPtr.ptr += increament * GetHandlingNum();

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&cbView[i], heapPtr);


	}

	return true;
}

bool Renderer::InitVBVResource(MeshData& mesh)
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

	desc.Width = sizeof(VertexData) * mesh.VertexDatas.size();

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
		IID_PPV_ARGS(mesh.vertexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "VBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapVB(MeshData& mesh)
{
	void* ptr = nullptr;

	hr = mesh.vertexBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "頂点データのマッピング失敗" << std::endl;

		return false;
	}
	memcpy(ptr, mesh.VertexDatas.data(), sizeof(VertexData) * mesh.VertexDatas.size());

	mesh.vertexBuffer->Unmap(0, nullptr);

	mesh.vbView.BufferLocation = mesh.vertexBuffer->GetGPUVirtualAddress();
	mesh.vbView.SizeInBytes = sizeof(VertexData) * mesh.VertexDatas.size();
	mesh.vbView.StrideInBytes = sizeof(VertexData);

	return true;
}

bool Renderer::InitIBVResource(MeshData& mesh)
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

	desc.Width = sizeof(int) * mesh.indices.size();

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
		IID_PPV_ARGS(mesh.indexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "IBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapIB(MeshData& mesh)
{
	void* ptr = nullptr;

	hr = mesh.indexBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "インデックスデータのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, mesh.indices.data(), sizeof(int) * mesh.indices.size());

	mesh.indexBuffer->Unmap(0, nullptr);

	mesh.ibView.BufferLocation = mesh.indexBuffer->GetGPUVirtualAddress();

	mesh.ibView.SizeInBytes = sizeof(int) * mesh.indices.size();;

	mesh.ibView.Format = DXGI_FORMAT_R32_UINT;

	return true;
}

bool Renderer::CreateRootSignature()
{
	{
		D3D12_ROOT_PARAMETER param[3] = {};

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

		param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

		param[2].Descriptor.ShaderRegister = 1;

		param[2].Descriptor.RegisterSpace = 0;

		D3D12_STATIC_SAMPLER_DESC desc_sampler = {};

		desc_sampler.Filter = D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;

		desc_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
	}

	return true;
}

bool Renderer::CreateRenderPipeline()
{
	D3D12_INPUT_ELEMENT_DESC desc_element[6] = {
	{"POSITION" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"TEXCOORD" , 0 , DXGI_FORMAT_R32G32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"TANGENT" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"NORMAL" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{ "BONEID",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "WEIGHT",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

	hr = D3DReadFileToBlob(L"VertexShader.cso", vs_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "頂点シェーダー読み込みエラー" << std::endl;

		return false;
	}

	hr = D3DReadFileToBlob(L"PixelShader.cso", ps_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ピクセルシェーダー読み込みエラー" << std::endl;

		return false;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

	desc_pipe.InputLayout = { desc_element , _countof(desc_element) };

	desc_pipe.pRootSignature = rootSig.Get();

	desc_pipe.VS = { vs_blob->GetBufferPointer() , vs_blob->GetBufferSize() };

	desc_pipe.PS = { ps_blob->GetBufferPointer() , ps_blob->GetBufferSize() };

	desc_pipe.RasterizerState = desc_ras;

	desc_pipe.BlendState = desc_blend;

	desc_pipe.DepthStencilState = DXRenderSystem::GetInstance().GetStencilDesc();

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

bool Renderer::InitBoneResource()
{
	UINT bufferSize = sizeof(XMMATRIX) * finalBoneMats.size();

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

	desc.Width = bufferSize;

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
		IID_PPV_ARGS(&boneBuffer)
	);

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapBB()
{
	for (int i = 0;i < finalBoneMats.size();i++)
	{
		if (boneInfos.size() > i)
		{
			finalBoneMats[i] = Utility::ConvertAiMatrix4x4ToXMMATRIX(boneInfos[i].finalMat);
		}
		else
		{
			finalBoneMats[i] = DirectX::XMMATRIX(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		}
	}

	void* ptr= nullptr;

	hr = boneBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ボーンの最終座標が収納されたBBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, finalBoneMats.data(), sizeof(XMMATRIX) * finalBoneMats.size());

	boneBuffer->Unmap(0, nullptr);

	return true;
}

bool Renderer::InitSRVForBoneBuffer()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = finalBoneMats.size();
	srvDesc.Buffer.StructureByteStride = sizeof(XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
	UINT increment = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	handle.ptr += increment * GetHandlingNum();

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(boneBuffer.Get(), &srvDesc, handle);

	return true;
}

std::unordered_map<string, UINT> boneMapping;

aiNodeAnim* ParseAnimNode(aiAnimation& anim, string nodeName)
{
	for (int i = 0;i < anim.mNumChannels;i++)
	{
		if (anim.mChannels[i]->mNodeName.C_Str() == nodeName)
		{
			return anim.mChannels[i];
		}
	}

	return nullptr;
}

UINT FindScalingKey(float animationTime ,const aiNodeAnim* nodeAnim)
{
	for (int i = 0;i < nodeAnim->mNumScalingKeys - 1;i++)
	{
		auto nextNode = nodeAnim->mScalingKeys[i + 1];

		if (animationTime < nextNode.mTime)
		{
			return i;
		}
	}
}

UINT FindQuaternionKey(float animationTime, const aiNodeAnim* nodeAnim)
{
	for (int i = 0;i < nodeAnim->mNumRotationKeys - 1;i++)
	{
		auto nextNode = nodeAnim->mRotationKeys[i + 1];

		if (animationTime < nextNode.mTime)
		{
			return i;
		}
	}
}

UINT FindPositionKey(float animationTime, const aiNodeAnim* nodeAnim)
{
	for (int i = 0;i < nodeAnim->mNumPositionKeys - 1;i++)
	{
		auto nextNode = nodeAnim->mPositionKeys[i + 1];

		if (animationTime < nextNode.mTime)
		{
			return i;
		}
	}
}

void interpolateScaling(aiVector3D& scale , float animationTime , const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumScalingKeys <= 1)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			scale = nodeAnim->mScalingKeys[0].mValue;
		}
		else 
		{
			scale = aiVector3D(1, 1, 1);
		}

		return;
	}

	auto currentKeyIndex = FindScalingKey(animationTime , nodeAnim);
	auto nextKeyIndex = currentKeyIndex + 1;
	auto curKey = nodeAnim->mScalingKeys[currentKeyIndex];
	auto nextKey = nodeAnim->mScalingKeys[nextKeyIndex];
	auto intervalTime = nextKey.mTime - curKey.mTime;
	auto factor = (animationTime - curKey.mTime) / intervalTime;
	auto curKeyVec = curKey.mValue;
	auto nextKeyVec = nextKey.mValue;
	auto subVec = nextKeyVec - curKeyVec;
	subVec *= factor;
	auto resultVec = curKeyVec + subVec;
	scale = resultVec;
}

void interpolateQuaternion(aiQuaternion& quat, float animationTime, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumRotationKeys <= 1)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			quat = nodeAnim->mRotationKeys[0].mValue;
		}
		else
		{
			quat = aiQuaternion(1, 0, 0, 0);
		}

		return;
	}

	auto currentKeyIndex = FindQuaternionKey(animationTime, nodeAnim);
	auto nextKeyIndex = currentKeyIndex + 1;
	auto curKey = nodeAnim->mRotationKeys[currentKeyIndex];
	auto nextKey = nodeAnim->mRotationKeys[nextKeyIndex];
	auto intervalTime = nextKey.mTime - curKey.mTime;
	auto factor = (animationTime - curKey.mTime) / intervalTime;
	auto curKeyQuat = curKey.mValue;
	auto nextKeyQuat = nextKey.mValue;
    aiQuaternion::Interpolate(quat , curKeyQuat, nextKeyQuat,factor);
	quat.Normalize();
}

void InterpolateTranslation(aiVector3D& trans, float animationTime, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumPositionKeys <= 1)
	{
		if (nodeAnim->mNumPositionKeys == 1)
		{
			trans = nodeAnim->mPositionKeys[0].mValue;
		}
		else
		{
			trans = aiVector3D(0, 0, 0);
		}

		return;
	}

	auto currentKeyIndex = FindPositionKey(animationTime, nodeAnim);
	auto nextKeyIndex = currentKeyIndex + 1;
	auto curKey = nodeAnim->mPositionKeys[currentKeyIndex];
	auto nextKey = nodeAnim->mPositionKeys[nextKeyIndex];
	auto intervalTime = nextKey.mTime - curKey.mTime;
	auto factor = (animationTime - curKey.mTime) / intervalTime;
	auto curKeyVec = curKey.mValue;
	auto nextKeyVec = nextKey.mValue;
	auto subVec = nextKeyVec - curKeyVec;
	subVec *= factor;
	auto resultVec = curKeyVec + subVec;
	trans = resultVec;
}

void Renderer::ReadHierarchyNode(float animationTime , aiNode& node , aiMatrix4x4& parentMat
 , aiAnimation& anim)
{
	string name(node.mName.C_Str());

	aiMatrix4x4 nodeTransformation(node.mTransformation);

	auto channel = ParseAnimNode(anim , name);

	aiMatrix4x4 resultMat;

	if (channel)
	{
		aiVector3D scaling;
		interpolateScaling(scaling , animationTime , channel);
		aiMatrix4x4 scalingMat;
		aiMatrix4x4::Scaling(scaling , scalingMat);

		aiQuaternion quat;
		interpolateQuaternion(quat, animationTime, channel);
		aiMatrix4x4 rotMat = aiMatrix4x4(quat.GetMatrix());

		aiVector3D trans;
		InterpolateTranslation(trans , animationTime , channel);
		aiMatrix4x4 transMat;
		aiMatrix4x4::Translation(trans, transMat);

		nodeTransformation = transMat * rotMat * scalingMat;
	}

	resultMat = parentMat * nodeTransformation;

	if (boneMapping.find(name) != boneMapping.end())
	{
		boneInfos[boneMapping[name]].finalMat = resultMat * boneInfos[boneMapping[name]].offset;

	}

	for (int i = 0;i < node.mNumChildren;i++)
	{
		ReadHierarchyNode(animationTime , *node.mChildren[i] , resultMat , anim);
	}

}

void Renderer::StartAnimation(float timeInSecond)
{

	aiAnimation* anim = scene.mAnimations[30];

	auto ticksPerSecond = anim->mTicksPerSecond;
	auto ticksInSecond = timeInSecond * ticksPerSecond;
	auto animationTime = fmod(ticksInSecond  / 3, anim->mDuration);


	aiMatrix4x4 mat;

	mat = aiMatrix4x4();

	ReadHierarchyNode(animationTime, *scene.mRootNode , mat, *anim);

	InitBoneResource();

	MapBB();
}

void Renderer::StoreVertices(const std::string filePath)
{
	aiVector3D zero = aiVector3D(0,0,0);

	UINT baseVertexNum = 0;
	UINT baseIndexNum = 0;

	for (int i = 0;i < scene.mNumMeshes; i++)
	{
		aiMesh* mesh = scene.mMeshes[i];

		meshDatas[i].VertexDatas.resize(mesh->mNumVertices);
		meshDatas[i].vertexBoneDatas.resize(mesh->mNumVertices);

		for (int j = 0;j < mesh->mNumVertices;j++)
		{
			VertexData* data = &meshDatas[i].VertexDatas[j];

			auto pos = mesh->mVertices[j];

			auto nor = mesh->mNormals[j];

			auto tan = mesh->HasTangentsAndBitangents() ? mesh->mTangents[j] : zero;

			auto coord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : zero;

			data->vertex = XMFLOAT3(pos.x, pos.y, pos.z);
			data->normal = XMFLOAT3(nor.x, nor.y, nor.z);
			data->tangent = XMFLOAT3(tan.x, tan.y, tan.z);
			data->texCoord = XMFLOAT2(coord.x, coord.y);
		}

		meshDatas[i].indices.resize(mesh->mNumFaces * 3);

		for (int j = 0;j < mesh->mNumFaces;j++)
		{
			auto face = mesh->mFaces[j];

			meshDatas[i].indices[j * 3 + 0] = face.mIndices[0];
			meshDatas[i].indices[j * 3 + 1] = face.mIndices[1];
			meshDatas[i].indices[j * 3 + 2] = face.mIndices[2];
		}

		meshDatas[i].baseVertexNumber = baseVertexNum;
		meshDatas[i].baseIndexNumber = baseIndexNum;

		baseVertexNum += mesh->mNumVertices;
		baseIndexNum += meshDatas[i].indices.size();

		if (scene.HasMaterials())
			InitMaterial( filePath, *mesh, meshDatas[i], i);

		if (mesh->HasBones())
		{
			for (UINT j = 0;j < mesh->mNumBones;j++)
			{
				auto bone = mesh->mBones[j];

				UINT boneIndex = 0;

				if (boneMapping.find(bone->mName.C_Str()) == boneMapping.end())
				{
					boneIndex = boneMapping.size();

				

					BoneInfo bi;

					boneInfos.emplace_back(bi);
				}
				else
				{
					boneIndex = boneMapping[bone->mName.C_Str()];
				}

				boneMapping[bone->mName.C_Str()] = boneIndex;

				boneInfos[boneIndex].offset = bone->mOffsetMatrix;

				for (UINT k = 0;k < bone->mNumWeights;k++)
				{
					auto weight = bone->mWeights[k];

					meshDatas[i].vertexBoneDatas[weight.mVertexId].boneID.emplace_back(boneIndex);
					meshDatas[i].vertexBoneDatas[weight.mVertexId].weight.emplace_back(weight.mWeight);
				}
			}

			for (int j = 0;j < mesh->mNumVertices;j++)
			{
				VertexData* data = &meshDatas[i].VertexDatas[j];

				auto& boneiD = meshDatas[i].vertexBoneDatas[j].boneID;
				auto& weight = meshDatas[i].vertexBoneDatas[j].weight;

				if (boneiD.size() == 0)
				{
					data->boneID = XMFLOAT4(0,0,0,0);
					data->weight = XMFLOAT4(0,0,0,0);
				}
				else if (boneiD.size() == 1)
				{
					data->boneID = XMFLOAT4(boneiD[0], 0, 0, 0);
					data->weight = XMFLOAT4(weight[0], 0, 0, 0);
				}
			    else if (boneiD.size() == 2)
				{
					data->boneID = XMFLOAT4(boneiD[0], boneiD[1], 0, 0);
					data->weight = XMFLOAT4(weight[0], weight[1], 0, 0);
				}
				else if (boneiD.size() == 3)
				{
					data->boneID = XMFLOAT4(boneiD[0], boneiD[1], boneiD[2], 0);
					data->weight = XMFLOAT4(weight[0], weight[1], weight[2], 0);
				}
				else
				{
					data->boneID = XMFLOAT4(boneiD[0], boneiD[1], boneiD[2], boneiD[3]);
					data->weight = XMFLOAT4(weight[0], weight[1], weight[2], weight[3]);
				}


			}
		}

		
	}
}

void Renderer::InitMaterial(const std::string filePath , const aiMesh& mesh , MeshData& data , const int meshIndex)
{
	auto index = filePath.find_last_of("/");

	std::string dir = filePath.substr(0, index);

	aiString path;

	aiMaterial* mat = scene.mMaterials[mesh.mMaterialIndex];

	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		std::string str = path.data;

		LoadTexture(dir + "/" + str, data);
	}
	else
	{
		LoadTexture("Textures/error.dds", data);
	}
}

bool Renderer::LoadMesh(const std::string filePath)
{
	GameManager::GetInstance().PauseTime();

	meshDatas.clear();

	int flag = 0;

	flag |= aiProcess_Triangulate;

	flag |= aiProcess_GenSmoothNormals;

	flag |= aiProcess_FlipUVs;

	flag |= aiProcess_JoinIdenticalVertices;

	scene = *import.ReadFile(filePath, flag);

	if (scene.HasMeshes())
	{
		meshDatas.resize(scene.mNumMeshes);

		StoreVertices(filePath);

		bool isSuccess = true;

		for (auto& mesh : meshDatas)
		{
			isSuccess &= InitVBVResource(mesh);

			isSuccess &= MapVB(mesh);

			isSuccess &= InitIBVResource(mesh);

			isSuccess &= MapIB(mesh);
		}

		if (!isSuccess)
		{
			std::cerr << "読み込みの初期化の失敗" << std::endl;

			return false;
		}

		GameManager::GetInstance().UnpauseTime();

		return true;
	}
	else
	{
		std::cout << "メッシュを持っていない" << std::endl;

		GameManager::GetInstance().UnpauseTime();

		return false;
	}
}

bool Renderer::LoadTexture(const std::string fileName, MeshData& mesh)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), -1, NULL, 0);
	std::wstring ws(len, L'\0');

	MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), -1, &ws[0], len);

	ws.pop_back();

	DirectX::ResourceUploadBatch batch(DXRenderSystem::GetInstance().GetDevice().Get());

	batch.Begin();

	hr = DirectX::CreateDDSTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, ws.c_str(), mesh.texView.resource.GetAddressOf(), true);

	std::wcout << L"path = " << ws << std::endl;

	if (FAILED(hr))
	{
		std::cerr << "ddsファイル読み込みエラー" << std::endl;

		hr = DirectX::CreateDDSTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, L"Textures/error.dds", mesh.texView.resource.GetAddressOf(), true);
	}

	auto load = batch.End(DXRenderSystem::GetInstance().GetCommandQueue().Get());

	load.wait();

	UINT descriptorSize = DXRenderSystem::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	int handlingNum = GetHandlingNum();

	// meshIndexは「現在のmesh番号」 (呼び出し元でi番目とか渡す)
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += handlingNum * descriptorSize;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += handlingNum * descriptorSize;

	// meshごとに異なるハンドル保存
	mesh.texView.cpu_handle = cpuHandle;
	mesh.texView.gpu_handle = gpuHandle;

	auto texture_desc = mesh.texView.resource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	desc.Format = texture_desc.Format;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.Texture2D.MipLevels = texture_desc.MipLevels;
	desc.Texture2D.PlaneSlice = 0;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(mesh.texView.resource.Get(), &desc, cpuHandle);

	return true;
}

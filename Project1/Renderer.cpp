#include "Renderer.h"
#include <d3dcompiler.h>
#include <ResourceUploadBatch.h>
#include <DDSTextureLoader.h>
#include <VertexTypes.h>
#include "Camera.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "DXRenderSystem.h"
#include "TextureLoader.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology)
{
	switch (topology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// テッセレーション用（patch）
	case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

	default:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
}

Renderer::Renderer(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

Renderer::~Renderer()
{
	auto& dxr = DXRenderSystem::GetInstance();

}

void Renderer::AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex)
{
	if (currentCBCount != DXRenderSystem::GetInstance().GetShadowMapNum() + 1)
	{
		currentCBCount = DXRenderSystem::GetInstance().GetShadowMapNum() + 1;

		InitCBVResource();

		LinkCBVResourceToDescHeap();
	}

	ConvertValues();

	Update();

	if (DirectX::XMVectorGetX(DirectX::XMVector3Length(attachFrom->GetWorldScale())) == 0)
		return;

	MapCB(backbufferIndex);

	MapMB();

	if (!DXRenderSystem::GetInstance().GetIsShadowMapRendering())
	{
		cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);
	}
	else
	{
		int baseOffset = 2;

		int shadowIndex = DXRenderSystem::GetInstance().GetCurrentShadowMapIndex();
		int bufferCount = 2;

		// 正しい計算式:
		// ベース + (ライト番号 × 1ライトあたりのバッファ数) + 現在のフレーム番号
		int targetIndex = baseOffset + (shadowIndex * bufferCount) + backbufferIndex;

		cmdList->SetGraphicsRootConstantBufferView(0, cbView[targetIndex].BufferLocation);
	}

	cmdList->SetGraphicsRootShaderResourceView(6, materialResource->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootDescriptorTable(8, DXRenderSystem::GetInstance().GetShadowMapTextureSRVGPUHandle());

	for (auto& func : cmdFuncs)
		func(cmdList);

	if (DXRenderSystem::GetInstance().GetIsShadowMapRendering())
		cmdList->SetPipelineState(shadowMapPipeline.Get());
	else
		cmdList->SetPipelineState(currentPipeline.Get());

	cmdList->IASetPrimitiveTopology(primitiveType);

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		auto& mesh = meshData->meshes[i];

		int texHandlesIndex = (i * 2) % texHandles.size();

		cmdList->SetGraphicsRootDescriptorTable(1, texHandles[texHandlesIndex]->gpuHandle);

		cmdList->IASetVertexBuffers(0, 1, &mesh.vbView);

		cmdList->IASetIndexBuffer(&mesh.ibView);

		AddCommand(cmdList);
		AddCommand(cmdList , i);

		cmdList->DrawIndexedInstanced(mesh.indices.size(), instanceCount, 0, 0, 0);
	}
}

void Renderer::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{

}

void Renderer::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList , UINT meshInd)
{

}

void Renderer::Init()
{
	bool isSuccess = true;

	isSuccess &= InitCBVResource();

	isSuccess &= LinkCBVResourceToDescHeap();

    ConvertValues();

	isSuccess &= MapCB(0);

	isSuccess &= CreateMesh(MeshLoader::BOX);

	PrepareForRenderPipeline();

	InitRenderer();

	isSuccess &= CreateRenderPipeline();

	if (!isSuccess)
	{
		std::cerr << "レンダーコンポネントの初期化失敗" << std::endl;
	}
}

bool Renderer::InitCBVResource()
{
	constantBuffer.resize(DXRenderSystem::frameBuffer * currentCBCount);

	for (int i = 0; i < constantBuffer.size(); i++)
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
	cbView.resize(constantBuffer.size());
	cbHandle.resize(constantBuffer.size());

	for (int i = 0; i < constantBuffer.size(); i++)
	{
		cbHandle[i] = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetHandle());

		cbView[i].BufferLocation = constantBuffer[i]->GetGPUVirtualAddress();

		cbView[i].SizeInBytes = sizeof(Transform);

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&cbView[i], cbHandle[i]->cpuHandle);
	}

	return true;
}


void Renderer::ConvertValues()
{
	trans.world = attachFrom->GetTRS();

	float colR = DirectX::XMVectorGetX(color);
	float colG = DirectX::XMVectorGetY(color);
	float colB = DirectX::XMVectorGetZ(color);
	float colA = DirectX::XMVectorGetW(color);

	trans.view = Camera::GetInstance().GetViewMat();

	trans.projection = Camera::GetInstance().GetProjMat();

	auto color_normalized = XMFLOAT4(colR, colG, colB, colA);

	trans.color = color_normalized;
}

bool Renderer::MapCB(int backBuffer)
{
	int frameIndex = backBuffer + 1;

	frameIndex = frameIndex % 2;

	int targetIndex = 0;

	if (DXRenderSystem::GetInstance().GetIsShadowMapRendering())
	{
		int shadowIndex = DXRenderSystem::GetInstance().GetCurrentShadowMapIndex();

		targetIndex = 2 + (shadowIndex * 2) + frameIndex;
	}
	else
	{
		targetIndex = frameIndex;
	}

	if (targetIndex >= constantBuffer.size())
	{
		std::cerr << "CBインデックス範囲外: " << targetIndex << std::endl;
		return false;
	}

	void* ptr = nullptr;
	HRESULT hr = constantBuffer[targetIndex]->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		return false;
	}

	memcpy(ptr, &trans, sizeof(Transform));

	return true;
}

bool Renderer::InitMaterialResource()
{
	UINT bufferSize = sizeof(Material) * mats.size();

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
		IID_PPV_ARGS(materialResource.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "マテリアルのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapMB()
{
	void* ptr = nullptr;

	hr = materialResource->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "マテリアルの情報が収納されたMBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, mats.data(), sizeof(Material) * mats.size());

	return true;
}

bool Renderer::InitSRVForMaterialResource()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = mats.size();
	srvDesc.Buffer.StructureByteStride = sizeof(Material);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	materialHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetHandle());

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(materialResource.Get(), &srvDesc, materialHandle->cpuHandle);

	return true;
}

void Renderer::CreateSRV(ComPtr<ID3D12Resource> texResource , D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	auto texture_desc = texResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	desc.Format = texture_desc.Format;
	desc.Texture2D.MipLevels = texture_desc.MipLevels;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0;
	desc.Texture2D.PlaneSlice = 0;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(texResource.Get(), &desc, handle);
}

void Renderer::RegisterTextureSRV()
{
	texHandles.clear();

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		auto list = DXRenderSystem::GetInstance().GetHeapManager()->GetSerialHandle(2);

		CreateSRV(meshData->meshes[i].texResource , list[0]->cpuHandle);

		auto desc = meshData->meshes[i].texResource->GetDesc();

		mats[i].TEXELSIZE.x = 1 / (float)desc.Width;
		mats[i].TEXELSIZE.y = 1 / (float)desc.Height;
		mats[i].TEXELSIZE.z = desc.Width;
		mats[i].TEXELSIZE.w = desc.Height;

		CreateSRV(meshData->meshes[i].normalTexResource , list[1]->cpuHandle);

		for (int i = 0;i < list.size();i++)
		{
			texHandles.push_back(std::move(list[i]));
		}
	}
}

void Renderer::ApplyTextureOnMesh(int idx , Renderer::TEXTURE_TYPE type , ComPtr<ID3D12Resource> texResource)
{
	auto texture_desc = texResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	desc.Format = texture_desc.Format;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.Texture2D.MipLevels = texture_desc.MipLevels;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0;
	desc.Texture2D.PlaneSlice = 0;

	int index = idx * TEXTURE_TYPE::CountLast + (int)type;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(texResource.Get(), &desc, texHandles[index]->cpuHandle);
}

void Renderer::ApplyTextureOnMesh(int idx, Renderer::TEXTURE_TYPE type, ComPtr<ID3D12Resource> texResource , D3D12_SHADER_RESOURCE_VIEW_DESC desc)
{
	auto texture_desc = texResource->GetDesc();

	int index = idx * TEXTURE_TYPE::CountLast + (int)type;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(texResource.Get(), &desc, texHandles[index]->cpuHandle);
}

std::type_index Renderer::GetBaseType()
{
	return typeid(Component);
}

bool Renderer::CreateMesh(const MeshLoader::MESHTYPE type)
{
	currentMeshType = type;

	meshData = MeshLoader::GetInstance().GetMesh(type);

	mats.clear();

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		mats.push_back(meshData->meshes[i].mat);
	}

	RegisterTextureSRV();

	InitInCreateMesh();

	InitMaterialResource();

	MapMB();

	InitSRVForMaterialResource();

	return true;
}

void Renderer::InitInCreateMesh()
{

}

void Renderer::Update()
{
}

void Renderer::InitRenderer()
{
}

void Renderer::AddCommandListFunction(std::function<void(ComPtr<ID3D12GraphicsCommandList>)> f)
{
	cmdFuncs.emplace_back(f);
}

void Renderer::SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY type)
{
	primitiveType = type;

	CreateRenderPipeline();
}

D3D12_PRIMITIVE_TOPOLOGY Renderer::GetPrimitiveType()
{
	return primitiveType;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC Renderer::GetDefaultPipelineDesc()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

	desc_pipe.InputLayout = { descElement.data() , static_cast<unsigned int>(descElement.size()) };

	desc_pipe.pRootSignature = DXRenderSystem::GetInstance().GetRootSignature().Get();

	desc_pipe.VS = { vsBlob->GetBufferPointer() , vsBlob->GetBufferSize() };

	desc_pipe.PS = { psBlob->GetBufferPointer() , psBlob->GetBufferSize() };

	desc_pipe.RasterizerState = descRas;

	desc_pipe.BlendState = descBlend;

	desc_pipe.DepthStencilState = dsDesc;

	desc_pipe.SampleMask = UINT_MAX;

	desc_pipe.PrimitiveTopologyType = GetTopologyType(primitiveType);

	desc_pipe.NumRenderTargets = 1;

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
	{
		desc_pipe.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}

	desc_pipe.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	desc_pipe.SampleDesc.Count = DXRenderSystem::msaaCount;

	desc_pipe.SampleDesc.Quality = DXRenderSystem::GetInstance().msaaQuality - 1;

	return desc_pipe;
}

void Renderer::AddInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc)
{
	descElement.emplace_back(desc);
}

void Renderer::SetRasterizerDesc(D3D12_RASTERIZER_DESC desc)
{
	descRas = desc;
}

void Renderer::SetRTBDesc(D3D12_RENDER_TARGET_BLEND_DESC desc)
{
	descRtBlend = desc;
}

std::vector<std::string> Renderer::GetMeshNameList()
{
	std::vector<std::string> list;

	for (auto& mesh : meshData->meshes)
	{
		list.push_back(mesh.meshName);
	}

	return list;
}

void Renderer::LoadVertexShader(LPCWSTR vsName)
{
	hr = D3DReadFileToBlob(vsName, vsBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "頂点シェーダー読み込みエラー" << std::endl;

		return;
	}

	if (isPipelineAlreadyCreated)
	{
		CreateRenderPipeline();
	}
}

void Renderer::LoadPixelShader(LPCWSTR psName)
{
	hr = D3DReadFileToBlob(psName, psBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ピクセルシェーダー読み込みエラー" << std::endl;

		return;
	}

	if (isPipelineAlreadyCreated)
	{
		CreateRenderPipeline();
	}
}

Material& Renderer::GetMaterial(int idx)
{
	return mats[idx];
}

void Renderer::SetMaterial(int idx , const Material& mat)
{
	mats[idx] = mat;
}
 
D3D12_GPU_DESCRIPTOR_HANDLE Renderer::GetTextureGPUHandle(int idx, Renderer::TEXTURE_TYPE type)
{
	return texHandles[idx * TEXTURE_TYPE::CountLast + (int)type]->gpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetTextureCPUHandle(int idx, Renderer::TEXTURE_TYPE type)
{
	return texHandles[idx * TEXTURE_TYPE::CountLast + (int)type]->cpuHandle;
}

void Renderer::SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC desc)
{
	dsDesc = desc;
}

void Renderer::PrepareForRenderPipeline()
{
	D3D12_INPUT_ELEMENT_DESC desc_element[9] = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,        0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 20,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 32,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 48,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	{ "BONEID",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 60,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "WEIGHT",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 76,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	{ "PARENT",    0, DXGI_FORMAT_R32_FLOAT,           0, 92,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "INDEX",    0, DXGI_FORMAT_R32_FLOAT,           0, 96,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	descElement.assign(std::begin(desc_element), std::end(desc_element));

	descRas = {};

	descRas.CullMode = D3D12_CULL_MODE_NONE;

	descRas.FillMode = D3D12_FILL_MODE_SOLID;

	descRas.FrontCounterClockwise = false;

	descRas.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;

	descRas.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;

	descRas.DepthClipEnable = false;

	descRas.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

	descRas.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	descRas.MultisampleEnable = false;

	descRas.AntialiasedLineEnable = false;

	descRas.ForcedSampleCount = 0;

	descRtBlend = {};

	descRtBlend.BlendEnable = true;
	descRtBlend.BlendOp = D3D12_BLEND_OP_ADD;
	descRtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	descRtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	descRtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	descRtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
	descRtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	descRtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	descRtBlend.LogicOpEnable = false;
	descRtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	descBlend = {};

	descBlend.AlphaToCoverageEnable = false;

	descBlend.IndependentBlendEnable = false;

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
	{
		descBlend.RenderTarget[i] = descRtBlend;
	}

	hr = D3DReadFileToBlob(L"RendererVertexShader.cso", vsBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "頂点シェーダー読み込みエラー" << std::endl;

		return;
	}

	hr = D3DReadFileToBlob(L"RendererPixelShader.cso", psBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ピクセルシェーダー読み込みエラー" << std::endl;

		return;
	}

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = false;
}

bool Renderer::CreateRenderPipeline()
{
	isPipelineAlreadyCreated = true;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = GetDefaultPipelineDesc();

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateGraphicsPipelineState(&desc_pipe, IID_PPV_ARGS(currentPipeline.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "パイプラインの作成失敗" << std::endl;

		return false;
	}

	pipeline.push_back(currentPipeline);

	desc_pipe.RasterizerState.DepthBias = 1;
	desc_pipe.RasterizerState.SlopeScaledDepthBias = 1;
	desc_pipe.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	desc_pipe.SampleDesc.Count = 1;
	desc_pipe.SampleDesc.Quality = 0;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateGraphicsPipelineState(&desc_pipe, IID_PPV_ARGS(shadowMapPipeline.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "シャドウマップ用パイプラインの作成失敗" << std::endl;

		return false;
	}

	pipeline.push_back(shadowMapPipeline);

	return true;
}

bool Renderer::AddRenderPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
{
	ComPtr<ID3D12PipelineState> pso;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "パイプラインの作成失敗" << std::endl;

		return false;
	}

	pipeline.push_back(pso);

	return true;
}

void Renderer::ChangeRenderPipeline(int idx)
{
	if (idx >= pipeline.size())
	{
		std::cerr << "指定したインデックスがパイプラインの配列数以上" << std::endl;
		return;
	}

	currentPipeline = pipeline[idx];
}

MeshLoader::MESHTYPE  Renderer::GetCurrentMeshType()
{
	return currentMeshType;
}

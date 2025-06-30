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

Renderer::Renderer(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

Renderer::~Renderer()
{
}

void Renderer::AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex)
{
	ConvertValues();

	Update();

	MapCB();

	MapLLICB();

	MapDLB();

	MapPLB();

	MapSLB();

	cmdList->SetGraphicsRootSignature(rootSig.Get());

	cmdList->SetDescriptorHeaps(1, heapManager.GetHeap().GetAddressOf());

	cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);

	cmdList->SetGraphicsRootConstantBufferView(2, lightInfoCBView.BufferLocation);

	cmdList->SetGraphicsRootShaderResourceView(3, DLBuffer->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootShaderResourceView(4, PLBuffer->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootShaderResourceView(5, SLBuffer->GetGPUVirtualAddress());

	cmdList->SetPipelineState(pipeline.Get());

	cmdList->IASetPrimitiveTopology(primitiveType);

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		auto& mesh = meshData->meshes[i];

		int texHandlesIndex = (i * 2) % texHandles.size();

		cmdList->SetGraphicsRootDescriptorTable(1, texHandles[texHandlesIndex]->gpuHandle);

		cmdList->IASetVertexBuffers(0, 1, &mesh.vbView);

		cmdList->IASetIndexBuffer(&mesh.ibView);

		AddCommand(cmdList);

		cmdList->DrawIndexedInstanced(mesh.indices.size(), instanceCount, 0, 0, 0);
	}
}

void Renderer::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{

}

void Renderer::Init()
{
	bool isSuccess = true;

	isSuccess &= InitCBVResource();

	isSuccess &= LinkCBVResourceToDescHeap();

    ConvertValues();

	isSuccess &= MapCB();

	isSuccess &= InitLightInfoCB();

	isSuccess &= LinkLightInfoCBVToDescHeap();

	isSuccess &= MapLLICB();

	isSuccess &= InitDirectionalLightBResource();

	isSuccess &= InitDLBV();

	isSuccess &= MapDLB();

	isSuccess &= InitPointLightBResource();

	isSuccess &= InitPLBV();

	isSuccess &= MapPLB();

	isSuccess &= InitSpotLightBResource();

	isSuccess &= InitSLBV();

	isSuccess &= MapSLB();

	isSuccess &= CreateMesh(MeshLoader::BOX);

	PrepareForRootSignature();

	PrepareForRenderPipeline();

	InitRenderer();

	isSuccess &= CreateRootSignature();

	isSuccess &= CreateRenderPipeline();

	if (!isSuccess)
	{
		std::cerr << "レンダーコンポネントの初期化失敗" << std::endl;
	}
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
	for (int i = 0; i < DXRenderSystem::frameBuffer; i++)
	{
		cbHandle[i] = std::move(heapManager.GetHandle());

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

	trans.view = XMMatrixLookAtLH(Camera::GetInstance().GetPosition(), Camera::GetInstance().GetTargetPos(), DirectX::XMVectorSet(0, 1, 0, 0));

	trans.projection = XMMatrixPerspectiveFovLH(Camera::fov, Camera::GetInstance().aspect, Camera::nearZ, Camera::farZ);

	auto color_normalized = XMFLOAT4(colR, colG, colB, colA);

	trans.color = color_normalized;
}

bool Renderer::MapCB()
{
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


bool Renderer::InitLightInfoCB()
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

	desc.Width = sizeof(LightInfo);

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
		IID_PPV_ARGS(lightInfoCB.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "CBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::LinkLightInfoCBVToDescHeap()
{
	lightInfoCBHandle = std::move(heapManager.GetHandle());

	lightInfoCBView.BufferLocation = lightInfoCB->GetGPUVirtualAddress();

	lightInfoCBView.SizeInBytes = sizeof(LightInfo);

	DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&lightInfoCBView, lightInfoCBHandle->cpuHandle);

	return true;
}

bool Renderer::MapLLICB()
{
	lightInfo.ambientLightColor = GameManager::GetInstance().GetAmbientColor();

	auto dirLNum = GameManager::GetInstance().GetComponentList<DirectionalLight>().size();

	lightInfo.DirectionalLightNum = DirectX::XMFLOAT4(dirLNum, dirLNum, dirLNum, dirLNum);

	auto pLNum = GameManager::GetInstance().GetComponentList<PointLight>().size();

	lightInfo.PointLightNum = DirectX::XMFLOAT4(pLNum, pLNum, pLNum, pLNum);

	auto sLNum = GameManager::GetInstance().GetComponentList<SpotLight>().size();

	lightInfo.SpotLightNum = DirectX::XMFLOAT4(sLNum, sLNum, sLNum, sLNum);

	lightInfo.cameraPos = Camera::GetInstance().GetPosition();

	void* ptr = nullptr;

	hr =  lightInfoCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "座標行列が収納されたCBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &lightInfo, sizeof(LightInfo));

    lightInfoCB->Unmap(0, nullptr);

	return true;
}

void Renderer::CreateSRV(ComPtr<ID3D12Resource> texResource)
{
	texHandles.push_back(std::move(heapManager.GetHandle()));

	auto texture_desc = texResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	desc.Format = texture_desc.Format;

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	desc.Texture2D.MipLevels = texture_desc.MipLevels;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.ResourceMinLODClamp = 0;
	desc.Texture2D.PlaneSlice = 0;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(texResource.Get(), &desc, texHandles[texHandles.size() - 1]->cpuHandle);
}

void Renderer::RegisterTextureSRV()
{
	texHandles.clear();

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		CreateSRV(meshData->meshes[i].texResource);
		CreateSRV(meshData->meshes[i].normalTexResource);
	}
}

bool Renderer::CreateMesh(const MeshLoader::MESHTYPE type)
{
	currentMeshType = type;

	meshData = MeshLoader::GetInstance().GetMesh(type);

	RegisterTextureSRV();

	InitInCreateMesh();

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

void Renderer::AddRootParam(D3D12_ROOT_PARAMETER param)
{
	this->param.emplace_back(param);
}

int Renderer::GetRootParamNum()
{
	return param.size();
}

void Renderer::AddInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc)
{
	descElement.emplace_back(desc);
}

void Renderer::SetDescSampler(D3D12_STATIC_SAMPLER_DESC desc)
{
	descSampler = desc;
}

void Renderer::SetRasterizerDesc(D3D12_RASTERIZER_DESC desc)
{
	descRas = desc;
}

void Renderer::SetRTBDesc(D3D12_RENDER_TARGET_BLEND_DESC desc)
{
	descRtBlend = desc;
}

void Renderer::LoadVertexShader(LPCWSTR vsName)
{
	hr = D3DReadFileToBlob(vsName, vsBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "頂点シェーダー読み込みエラー" << std::endl;

		return;
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
}

void Renderer::SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC desc)
{
	dsDesc = desc;
}

void Renderer::PrepareForRootSignature()
{
	param.resize(6);

	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[0].Descriptor.ShaderRegister = 0;

	param[0].Descriptor.RegisterSpace = 0;

	rangeSRV[0].NumDescriptors = 1;

	rangeSRV[0].BaseShaderRegister = 0;

	rangeSRV[0].RegisterSpace = 0;

	rangeSRV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rangeSRV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	rangeSRV[1].NumDescriptors = 1;

	rangeSRV[1].BaseShaderRegister = 1;

	rangeSRV[1].RegisterSpace = 0;

	rangeSRV[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rangeSRV[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[1].DescriptorTable.NumDescriptorRanges = 2;

	param[1].DescriptorTable.pDescriptorRanges = rangeSRV;

	param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[2].Descriptor.ShaderRegister = 1;

	param[2].Descriptor.RegisterSpace = 0;

	param[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[3].Descriptor.ShaderRegister = 10;

	param[3].Descriptor.RegisterSpace = 0;

	param[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[4].Descriptor.ShaderRegister = 11;

	param[4].Descriptor.RegisterSpace = 0;

	param[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[5].Descriptor.ShaderRegister = 12;

	param[5].Descriptor.RegisterSpace = 0;

	descSampler = {};

	descSampler.Filter = D3D12_FILTER_ANISOTROPIC;

	descSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	descSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	descSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	descSampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	descSampler.MaxAnisotropy = 4;
	descSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	descSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	descSampler.MaxLOD = D3D12_FLOAT32_MAX;
	descSampler.MinLOD = -D3D12_FLOAT32_MAX;
	descSampler.ShaderRegister = 0;
	descSampler.RegisterSpace = 0;
	descSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void Renderer::PrepareForRenderPipeline()
{
	D3D12_INPUT_ELEMENT_DESC desc_element[8] = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,        0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 20,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 32,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 48,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	{ "BONEID",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 60,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "WEIGHT",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 76,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	{ "PARENT",    0, DXGI_FORMAT_R32_FLOAT,           0, 92,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } 
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

bool Renderer::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	desc.NumParameters = param.size();

	desc.NumStaticSamplers = 1;

	desc.pParameters = param.data();

	desc.pStaticSamplers = &descSampler;

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

bool Renderer::CreateRenderPipeline()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

	desc_pipe.InputLayout = { descElement.data() , static_cast<unsigned int>(descElement.size())};

	desc_pipe.pRootSignature = rootSig.Get();

	desc_pipe.VS = { vsBlob->GetBufferPointer() , vsBlob->GetBufferSize() };

	desc_pipe.PS = { psBlob->GetBufferPointer() , psBlob->GetBufferSize() };

	desc_pipe.RasterizerState = descRas;

	desc_pipe.BlendState = descBlend;

	desc_pipe.DepthStencilState = dsDesc;

	desc_pipe.SampleMask = UINT_MAX;

	desc_pipe.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	desc_pipe.NumRenderTargets = 1;

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
	{
		desc_pipe.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}

	desc_pipe.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	desc_pipe.SampleDesc.Count = DXRenderSystem::msaaCount;

	desc_pipe.SampleDesc.Quality = DXRenderSystem::GetInstance().msaaQuality - 1;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateGraphicsPipelineState(&desc_pipe, IID_PPV_ARGS(pipeline.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "パイプラインの作成失敗" << std::endl;

		return false;
	}

	return true;
}

MeshLoader::MESHTYPE  Renderer::GetCurrentMeshType()
{
	return currentMeshType;
}

bool Renderer::InitDirectionalLightBResource()
{
	DirLights.resize(100);

	UINT bufferSize = sizeof(DirLight) * DirLights.size();

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
		IID_PPV_ARGS(DLBuffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapDLB()
{
	auto list = GameManager::GetInstance().GetComponentList<DirectionalLight>();

	for (int i = 0; i< list.size() ; i++)
	{
		DirLight l;
		DirectX::XMStoreFloat3(&l.color , list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.direct, list[i]->GetLightDir());
		l.power = list[i]->GetPower();

		DirLights[i] = l;
	}

	void* ptr = nullptr;

	hr = DLBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ダイレクショナルライトのDLBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, DirLights.data(), sizeof(DirLight) * DirLights.size());

	DLBuffer->Unmap(0, nullptr);

	return true;
}

bool Renderer::InitDLBV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = DirLights.size();
	srvDesc.Buffer.StructureByteStride = sizeof(DirLight);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	DLBHandle = std::move(heapManager.GetHandle());

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(DLBuffer.Get(), &srvDesc, DLBHandle->cpuHandle);

	return true;
}


bool Renderer::InitPointLightBResource()
{
	PLights.resize(100);

	UINT bufferSize = sizeof(PLight) * PLights.size();

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
		IID_PPV_ARGS(PLBuffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapPLB()
{
	auto list = GameManager::GetInstance().GetComponentList<PointLight>();

	for (int i = 0; i < list.size(); i++)
	{
		PLight l;
		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.pos, list[i]->GetPosition());
		l.power = list[i]->GetPower();
		l.radius = list[i]->GetRadius();

	    PLights[i] = l;
	}

	void* ptr = nullptr;

	hr = PLBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポイントライトのPLBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, PLights.data(), sizeof(PLight) * PLights.size());

	PLBuffer->Unmap(0, nullptr);

	return true;
}

bool Renderer::InitPLBV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = PLights.size();
	srvDesc.Buffer.StructureByteStride = sizeof(PLight);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	PLBHandle = std::move(heapManager.GetHandle());

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(PLBuffer.Get(), &srvDesc, PLBHandle->cpuHandle);

	return true;
}

bool Renderer::InitSpotLightBResource()
{
	SLights.resize(100);

	UINT bufferSize = sizeof(SLight) * SLights.size();

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
		IID_PPV_ARGS(SLBuffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapSLB()
{
	auto list = GameManager::GetInstance().GetComponentList<SpotLight>();

	for (int i = 0; i < list.size(); i++)
	{
		SLight l;
		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.pos, list[i]->GetPosition());
		DirectX::XMStoreFloat3(&l.direct, list[i]->GetLightDir());
		l.length = list[i]->GetLength();
		l.angular = list[i]->GetAngular();
		l.power = list[i]->GetPower();

		SLights[i] = l;
	}

	void* ptr = nullptr;

	hr = SLBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポイントライトのPLBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, SLights.data(), sizeof(PLight) * SLights.size());

	SLBuffer->Unmap(0, nullptr);

	return true;
}

bool Renderer::InitSLBV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = SLights.size();
	srvDesc.Buffer.StructureByteStride = sizeof(SLight);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	SLBHandle = std::move(heapManager.GetHandle());

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(SLBuffer.Get(), &srvDesc, SLBHandle->cpuHandle);

	return true;
}


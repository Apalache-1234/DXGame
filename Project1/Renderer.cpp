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
	ConvertValues();

	Update();

	MapCB();

	MapLLICB();

	MapDLB();

	MapPLB();

	MapSLB();

	MapMB();

	MapTICB();

	cmdList->SetGraphicsRootSignature(rootSig.Get());

	cmdList->SetDescriptorHeaps(1, heapManager.GetHeap().GetAddressOf());

	cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);

	cmdList->SetGraphicsRootConstantBufferView(2, lightInfoCBView.BufferLocation);

	cmdList->SetGraphicsRootShaderResourceView(3, DLBuffer->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootShaderResourceView(4, PLBuffer->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootShaderResourceView(5, SLBuffer->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootShaderResourceView(6, materialResource->GetGPUVirtualAddress());

	cmdList->SetGraphicsRootConstantBufferView(7, timeInfoCBView.BufferLocation);

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

	isSuccess &= MapCB();

	isSuccess &= InitLightInfoCB();

	isSuccess &= LinkLightInfoCBVToDescHeap();

	isSuccess &= MapLLICB();

	isSuccess &= InitTimeInfoCB();

	isSuccess &= LinkTimeInfoCBVToDescHeap();

	isSuccess &= MapTICB();

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

	trans.view = Camera::GetInstance().GetViewMat();

	trans.projection = Camera::GetInstance().GetProjMat();

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
	lightInfo.ambientLightColor = DXRenderSystem::GetInstance().GetAmbientColor();

	auto dirLNum = SystemManager::GetInstance().GetComponentList<DirectionalLight>().size();

	lightInfo.DirectionalLightNum = DirectX::XMFLOAT4(dirLNum, dirLNum, dirLNum, dirLNum);

	auto pLNum = SystemManager::GetInstance().GetComponentList<PointLight>().size();

	lightInfo.PointLightNum = DirectX::XMFLOAT4(pLNum, pLNum, pLNum, pLNum);

	auto sLNum = SystemManager::GetInstance().GetComponentList<SpotLight>().size();

	lightInfo.SpotLightNum = DirectX::XMFLOAT4(sLNum, sLNum, sLNum, sLNum);

	lightInfo.cameraPos = Camera::GetInstance().GetPosition();

	lightInfo.cascadeShadowCount = DXRenderSystem::GetInstance().GetCascadeShadowCount();
	lightInfo.cascadeShadowDist = DXRenderSystem::GetInstance().GetCascadeShadowDist();

	DirectX::XMStoreFloat3(&lightInfo.fogColor , DXRenderSystem::GetInstance().GetFogColor());

	lightInfo.fogRange = DXRenderSystem::GetInstance().GetFogRange();

	lightInfo.fogFlag = DXRenderSystem::GetInstance().fogFlag;

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

bool Renderer::InitTimeInfoCB()
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

	desc.Width = sizeof(TimeInfo);

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
		IID_PPV_ARGS(timeInfoCB.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "時間の情報を持ったTIBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::LinkTimeInfoCBVToDescHeap()
{
	timeInfoCBHandle = std::move(heapManager.GetHandle());

	timeInfoCBView.BufferLocation = timeInfoCB->GetGPUVirtualAddress();

	timeInfoCBView.SizeInBytes = sizeof(TimeInfo);

	DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&timeInfoCBView, timeInfoCBHandle->cpuHandle);

	return true;
}

bool Renderer::MapTICB()
{
	auto system = SystemManager::GetInstance();

	timeInfo.GameTime = system.GetGameTime();
	timeInfo.GameDeltaTime = system.GetGameDeltaTime();
	timeInfo.RealTime = system.GetRealTime();
	timeInfo.RealDeltaTime = system.GetRealDeltaTime();

	void* ptr = nullptr;

	hr = timeInfoCB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "時間の情報が収納されたCBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &timeInfo, sizeof(TimeInfo));

	timeInfoCB->Unmap(0, nullptr);

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

	materialResource->Unmap(0, nullptr);

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

	materialHandle = std::move(heapManager.GetHandle());

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
		auto list = heapManager.GetSerialHandle(2);

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

void Renderer::AddRootParam(D3D12_ROOT_PARAMETER param)
{
	this->param.emplace_back(param);
}

void Renderer::AddCommandListFunction(std::function<void(ComPtr<ID3D12GraphicsCommandList>)> f)
{
	cmdFuncs.emplace_back(f);
}

int Renderer::GetRootParamNum()
{
	return param.size();
}

void Renderer::SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY type)
{
	primitiveType = type;

	CreateRootSignature();

	CreateRenderPipeline();
}

D3D12_PRIMITIVE_TOPOLOGY Renderer::GetPrimitiveType()
{
	return primitiveType;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC Renderer::GetDefaultPipelineDesc()
{
	if (rootSig == nullptr)
		CreateRootSignature();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

	desc_pipe.InputLayout = { descElement.data() , static_cast<unsigned int>(descElement.size()) };

	desc_pipe.pRootSignature = rootSig.Get();

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

void Renderer::SetDescSampler(D3D12_STATIC_SAMPLER_DESC desc , SamplerType type)
{
	int num = (int)type;

	descSamplers[num] = desc;
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
		CreateRootSignature();

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
		CreateRootSignature();

		CreateRenderPipeline();
	}
}

DescriptorHeapManager* Renderer::GetHeapPointer()
{
	return &heapManager;
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

void Renderer::PrepareForRootSignature()
{
	param.resize(9);

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

	param[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[6].Descriptor.ShaderRegister = 30;

	param[6].Descriptor.RegisterSpace = 0;

	param[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[7].Descriptor.ShaderRegister = 2;

	param[7].Descriptor.RegisterSpace = 0;

	shadowMapSRV[0].NumDescriptors = 1;

	shadowMapSRV[0].BaseShaderRegister = 100;

	shadowMapSRV[0].RegisterSpace = 0;

	shadowMapSRV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	shadowMapSRV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	param[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	param[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[8].DescriptorTable.NumDescriptorRanges = 1;

	param[8].DescriptorTable.pDescriptorRanges = shadowMapSRV;

	D3D12_STATIC_SAMPLER_DESC desSam;

	desSam = {};

	desSam.Filter = D3D12_FILTER_ANISOTROPIC;

	desSam.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	desSam.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	desSam.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desSam.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	desSam.MaxAnisotropy = 4;
	desSam.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	desSam.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	desSam.MaxLOD = D3D12_FLOAT32_MAX;
	desSam.MinLOD = -D3D12_FLOAT32_MAX;
	desSam.ShaderRegister = 0;
	desSam.RegisterSpace = 0;
	desSam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descSamplers.push_back(desSam);

	desSam = {};

	desSam.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	desSam.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	desSam.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	desSam.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	desSam.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	desSam.MaxAnisotropy = 4;
	desSam.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	desSam.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	desSam.MaxLOD = D3D12_FLOAT32_MAX;
	desSam.MinLOD = -D3D12_FLOAT32_MAX;
	desSam.ShaderRegister = 1;
	desSam.RegisterSpace = 0;
	desSam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descSamplers.push_back(desSam);
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

bool Renderer::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	desc.NumParameters = param.size();

	desc.NumStaticSamplers = descSamplers.size();

	desc.pParameters = param.data();

	desc.pStaticSamplers = descSamplers.data();

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
		std::cerr << "ダイレクショナルライトのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapDLB()
{
	auto list = SystemManager::GetInstance().GetComponentList<DirectionalLight>();;

	for (int i = 0; i< list.size() ; i++)
	{
		DirLight l;

		DirectX::XMStoreFloat3(&l.color , list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.direct, list[i]->GetLightDir());
	    l.shadow = list[i]->GetIsCastShadow();
		l.power = list[i]->GetPower();
		l.shadowMapIndex = list[i]->GetShadowMapIndex();
		for (int j = 0;j < DXRenderSystem::GetInstance().GetCascadeShadowCount();j++)
			l.matVP[j] = list[i]->lightVP[j];
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
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
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
	auto list = SystemManager::GetInstance().GetComponentList<PointLight>();

	for (int i = 0; i < list.size(); i++)
	{
		PLight l;
		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.pos, list[i]->GetPosition());
		l.power = list[i]->GetPower();
		l.radius = list[i]->GetRadius();
		l.shadow = list[i]->GetIsCastShadow();
		l.shadowMapIndex = list[i]->GetShadowMapIndex();

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
	auto list = SystemManager::GetInstance().GetComponentList<SpotLight>();

	for (int i = 0; i < list.size(); i++)
	{
		SLight l;
		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.pos, list[i]->GetPosition());
		DirectX::XMStoreFloat3(&l.direct, list[i]->GetLightDir());
		l.length = list[i]->GetLength();
		l.angular = list[i]->GetAngular();
		l.power = list[i]->GetPower();
		l.shadow = list[i]->GetIsCastShadow();
		l.shadowMapIndex = list[i]->GetShadowMapIndex();
		l.matVP = list[i]->lightVP;
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


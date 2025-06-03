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

	cmdList->SetGraphicsRootSignature(rootSig.Get());

	cmdList->SetDescriptorHeaps(1, heapManager.GetHeap().GetAddressOf());

	cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);

	cmdList->SetPipelineState(pipeline.Get());

	cmdList->IASetPrimitiveTopology(primitiveType);

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		auto& mesh = meshData->meshes[i];

		int texHandlesIndex = i % texHandles.size();

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
	auto position_mat = XMMatrixTranslationFromVector(attachFrom->GetWorldPosition());

	auto qWorld = attachFrom->GetWorldQuaternion();
	auto qLocal = attachFrom->GetLocalQuaternion();

	auto rotation_mat = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionMultiply(qLocal, qWorld));

	auto scale_mat = XMMatrixScalingFromVector(attachFrom->GetScale());

	trans.world = scale_mat * rotation_mat * position_mat;

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

void Renderer::RegisterTextureSRV()
{
	texHandles.clear();

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		texHandles.push_back(std::move(heapManager.GetHandle()));

		auto texture_desc = meshData->meshes[i].texResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		desc.Format = texture_desc.Format;

		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		desc.Texture2D.MipLevels = texture_desc.MipLevels;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.ResourceMinLODClamp = 0;
		desc.Texture2D.PlaneSlice = 0;

		DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(meshData->meshes[i].texResource.Get(), &desc, texHandles[i]->cpuHandle);
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
	param.resize(2);

	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[0].Descriptor.ShaderRegister = 0;

	param[0].Descriptor.RegisterSpace = 0;

	range = {};

	range.NumDescriptors = 1;

	range.BaseShaderRegister = 0;

	range.RegisterSpace = 0;

	range.OffsetInDescriptorsFromTableStart = 0;

	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	param[1].DescriptorTable.NumDescriptorRanges = 1;

	param[1].DescriptorTable.pDescriptorRanges = &range;

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

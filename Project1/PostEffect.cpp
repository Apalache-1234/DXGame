#include "PostEffect.h"

PostEffect::PostEffect(shared_ptr<Entity> ent) : Renderer::Renderer(ent)
{
}

PostEffect::~PostEffect()
{
}

std::type_index PostEffect::GetBaseType()
{
	return typeid(Renderer);
}

void PostEffect::InitRenderer()
{
	LoadVertexShader(L"PostEffectVertexShader.cso");

	CreateMesh(MeshLoader::PLANE);

	InitPostEffect();

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = {};
	dsDesc.BackFace = {};

	SetDepthStencilDesc(dsDesc);

	D3D12_STATIC_SAMPLER_DESC desSam;

	desSam = {};

	desSam.Filter = D3D12_FILTER_ANISOTROPIC;

	desSam.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desSam.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
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
}

void PostEffect::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->SetGraphicsRootConstantBufferView(9, postEffectCBView.BufferLocation);
}

void PostEffect::InitPostEffect()
{
}

bool PostEffect::MapPECB()
{
	return true;
}

void PostEffect::Update()
{
	MapPECB();
}
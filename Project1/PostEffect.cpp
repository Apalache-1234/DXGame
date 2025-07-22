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
}

void PostEffect::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->SetGraphicsRootConstantBufferView(paramNum, postEffectCBView.BufferLocation);
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
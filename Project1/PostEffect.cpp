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
}

void PostEffect::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	MapPECB();

	cmdList->SetGraphicsRootConstantBufferView(paramNum, postEffectCBView.BufferLocation);
}

void PostEffect::InitPostEffect()
{
}

bool PostEffect::MapPECB()
{
	return true;
}

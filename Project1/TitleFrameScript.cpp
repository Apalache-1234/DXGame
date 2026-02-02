#include "TitleFrameScript.h"

TitleFrameScript::TitleFrameScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TitleFrameScript::~TitleFrameScript()
{
}

void TitleFrameScript::OnStart()
{
	rend = attachFrom->AddComponent<SkinnedRenderer>();

	rend->CreateMesh(MeshLoader::FRAME);

	attachFrom->SetWorldPosition(DirectX::XMVectorSet(-1.1f,1.2f,0.25f,0));
	attachFrom->SetWorldScale(DirectX::XMVectorSet(5,5,5,0));
	attachFrom->SetRotation(DirectX::XMVectorSet(0,80,0,0));
}

void TitleFrameScript::OnUpdate()
{
}

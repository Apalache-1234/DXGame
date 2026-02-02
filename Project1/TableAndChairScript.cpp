#include "TableAndChairScript.h"
#include "MeshLoader.h"

TableAndChairScript::TableAndChairScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TableAndChairScript::~TableAndChairScript()
{
}

void TableAndChairScript::OnStart()
{
	rend = attachFrom->AddComponent<SkinnedRenderer>();

	rend->CreateMesh(MeshLoader::TABLE_AND_CHAIR);
	attachFrom->SetWorldPosition(DirectX::XMVectorSet(0, 0.4f, 1.350f, 0));
}

void TableAndChairScript::OnUpdate()
{
	attachFrom->SetRotation(DirectX::XMVectorSet(0, systemManager->GetGameTime(), 0, 0));
}

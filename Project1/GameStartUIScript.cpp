#include "GameStartUIScript.h"
#include "Entity.h"
#include "UIRenderer.h"

GameStartUIScript::GameStartUIScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

GameStartUIScript::~GameStartUIScript()
{
}

void GameStartUIScript::OnStart()
{
	std::cout << "ssss" << std::endl;
	attachFrom->AddComponent<UIRenderer>();
}

#include "ScriptComponent.h"

ScriptComponent::ScriptComponent(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
	tweenSystem = &TweenSystem::GetInstance();
	systemManager = &SystemManager::GetInstance();
	meshLoader = &MeshLoader::GetInstance();
	textureLoader = &TextureLoader::GetInstance();
	sceneManager = &SceneManager::GetInstance();
	camera = &Camera::GetInstance();
}

ScriptComponent::~ScriptComponent()
{
}

void ScriptComponent::OnMousePointerIn()
{
}

void ScriptComponent::OnMousePointerOn()
{
}

void ScriptComponent::OnMousePointerClick()
{
}

void ScriptComponent::OnMousePointerOut()
{
}

void ScriptComponent::OnStart()
{
}

void ScriptComponent::OnUpdate()
{
}

void ScriptComponent::OnFixedUpdate()
{
}

void ScriptComponent::OnDestroy()
{
}

void ScriptComponent::Init()
{
	OnStart();
}

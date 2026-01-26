#include "ScriptComponent.h"

ScriptComponent::ScriptComponent(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
	OnStart();
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

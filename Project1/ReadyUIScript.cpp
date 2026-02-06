#include "ReadyUIScript.h"
#include "UIRenderer.h"

ReadyUIScript::ReadyUIScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

ReadyUIScript::~ReadyUIScript()
{
}

void ReadyUIScript::OnStart()
{
	sRend = attachFrom.lock()->AddComponent<UIRenderer>();

	sRend->SetImageWidth(Window::width);
	sRend->SetImageHeight(Window::height);

	sRend->CreateTexture("Textures/READY.png");

	sRend->LoadPixelShader(L"ReadyUIPixelShader.cso");
}

void ReadyUIScript::OnUpdate()
{
}

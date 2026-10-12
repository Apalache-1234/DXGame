#include "PCH/pch.h"
#include "TitleNoiseScript.h"

TitleNoiseScript::TitleNoiseScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TitleNoiseScript::~TitleNoiseScript()
{
}

void TitleNoiseScript::OnStart()
{
	noisePE = systemManager->GetComponentList<PENoiseFilter>().back();

	noisePE->flag = true;
	noisePE->onUI = true;

	tw = tweenSystem->DoTween(1.0f, 0.0f , EasingType::EaseLine , 1.5f);
}

void TitleNoiseScript::OnUpdate()
{
	noisePE->SetIntensify(tw->GetValue());
}

#include "TitleSpotLightScript.h"

TitleSpotLightScript::TitleSpotLightScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TitleSpotLightScript::~TitleSpotLightScript()
{
}

void TitleSpotLightScript::OnStart()
{
	attachFrom->AddComponent<SpotLight>();

	slComp = attachFrom->GetComponent<SpotLight>();

	slComp->SetRotation(DirectX::XMVectorSet(70,0,0,0));
	slComp->SetPower(2);
	slComp->SetAngular(DirectX::XMConvertToRadians(60));
	slComp->SetIsCastShadow(true);
}

void TitleSpotLightScript::LightUpward()
{
	tw2 = tweenSystem->DoTween(slComp->GetRotation(), DirectX::XMVectorSet(180, 0, 0, 0), EasingType::EaseOutQuint, 6);
}

void TitleSpotLightScript::OnUpdate()
{
	if(tw == nullptr)
		tw = tweenSystem->DoTween(DirectX::XMVectorAdd(camera->GetPosition(), DirectX::XMVectorSet(0, 2, -5, 0)), DirectX::XMVectorAdd(camera->GetPosition(), DirectX::XMVectorSet(0, 2, 0, 0)) , EasingType::EaseOutQuint , 6);

	if (tw != nullptr)
		attachFrom->SetWorldPosition(tw->GetValue());

	if (tw2 != nullptr)
		slComp->SetRotation(tw2->GetValue());

}

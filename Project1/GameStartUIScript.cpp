#include "GameStartUIScript.h"
#include "Entity.h"
#include "UIRenderer.h"
#include "PEMosaicFilter.h"
#include "PERadicalBlur.h"
#include "KeyInput.h"

GameStartUIScript::GameStartUIScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

GameStartUIScript::~GameStartUIScript()
{
}

std::shared_ptr<Tween < XMVECTOR>> tw;
std::shared_ptr<Tween < float>> tw2;

std::shared_ptr<PERadicalBlur> mF;

void GameStartUIScript::OnStart()
{
	attachFrom->AddComponent<UIRenderer>();
	mF =  systemManager->GetComponentList<PERadicalBlur>().back();

	mF->flag = true;

}

void GameStartUIScript::OnUpdate()
{
	if (KeyInput::GetInstance().GetKey(B))
	{
		tw = tweenSystem->DoTween(DirectX::XMVectorSet(1000, 0, 0, 0), DirectX::XMVectorZero());
		tw2 = tweenSystem->DoTween(0.0f, 1.0f);
	}
	

	if (tw != nullptr)
	{
		if (!tw->GetIsEnd())
			attachFrom->SetWorldPosition(tw->GetValue());

		mF->SetBlurStrength(tw2->GetValue());
	}

}

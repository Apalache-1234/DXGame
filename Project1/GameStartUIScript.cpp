#include "GameStartUIScript.h"
#include "Entity.h"
#include "UIRenderer.h"
#include "PEMosaicFilter.h"
#include "KeyInput.h"
#include "UIRenderer.h"

GameStartUIScript::GameStartUIScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

GameStartUIScript::~GameStartUIScript()
{
}

void GameStartUIScript::OnStart()
{
	speedUIs.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("SUI"));
	speedUIs.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("PUI"));
	speedUIs.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("EUI"));
	speedUIs.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("EUI2"));
	speedUIs.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("DUI"));
	BlackScreen = sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("BlackScreen");

	speedUIs[0]->GetComponent<UIRenderer>()->CreateTexture("Textures/SpeedAB/S.png");
	speedUIs[1]->GetComponent<UIRenderer>()->CreateTexture("Textures/SpeedAB/P.png");
	speedUIs[2]->GetComponent<UIRenderer>()->CreateTexture("Textures/SpeedAB/E.png");
	speedUIs[3]->GetComponent<UIRenderer>()->CreateTexture("Textures/SpeedAB/E.png");
	speedUIs[4]->GetComponent<UIRenderer>()->CreateTexture("Textures/SpeedAB/D.png");
	BlackScreen->GetComponent<UIRenderer>()->color = DirectX::XMVectorSet(0, 0, 0, 0);
	BlackScreen->GetComponent<UIRenderer>()->SetImageWidth(Window::width);
	BlackScreen->GetComponent<UIRenderer>()->SetImageHeight(Window::height);

	tw = tweenSystem->DoTween(0.0f, 2.0f , EasingType::EaseLine , 3);
}

void GameStartUIScript::OnBlack()
{
	std::function<void()> f = [=]() { sceneManager->ChangeGameScene("CardGameScene"); };

	tw2 = tweenSystem->DoTween(DirectX::XMVectorSet(0, 0, 0, 0), DirectX::XMVectorSet(0, 0, 0, 2), EasingType::EaseLine, 2);

	tw2->OnComplete(f);
}


void GameStartUIScript::OnUpdate()
{
	for (int i = 0;i<speedUIs.size();i++)
	{
		speedUIs[i]->SetWorldPosition(DirectX::XMVectorSet( + 170 * (i - 2), std::sin(systemManager->GetRealTime() + i) * 10 + 340, 0, 0));
		speedUIs[i]->SetWorldScale(DirectX::XMVectorSet(0.3f, 0.3f, 0.3f,1));
		speedUIs[i]->GetComponent<UIRenderer>()->color = DirectX::XMVectorSet(1,1,1,tw->GetValue() - 0.2 * i);
	}

	if(tw2!=nullptr)
		BlackScreen->GetComponent<UIRenderer>()->color = tw2->GetValue();;
}

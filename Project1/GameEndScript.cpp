#include "GameEndScript.h"
#include "PERadicalBlur.h"
#include "GameManagerScript.h"
#include "GameManagerScript.h"
#include "UIRenderer.h"

GameEndScript::GameEndScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

GameEndScript::~GameEndScript()
{
}

void GameEndScript::OnStart()
{
	auto ent = systemManager->GetEntityByName("GameManager");

	gameM = ent->GetComponent<GameManagerScript>();

	GameFinished = sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("GameFinished");
	GameFinishedRend = GameFinished->AddComponent<UIRenderer>();
	GameFinishedRend->CreateTexture("Textures/GAMESFINISHED.png");
	GameFinishedRend->color = DirectX::XMVectorSet(1, 1, 1, 0);

	upBlackLine = sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("UpBlackLine");
	upBlackLineRend = upBlackLine->AddComponent<UIRenderer>();
	downBlackLine = sceneManager->GetCurrentGameScene()->CreateEntity<UIRenderer>("DownBlackLine");
	downBlackLineRend = downBlackLine->AddComponent<UIRenderer>();

	upBlackLineRend->color = DirectX::XMVectorSet(0,0,0,1);
	downBlackLineRend->color = DirectX::XMVectorSet(0,0,0,1);
	upBlackLineRend->SetImageHeight(Window::height);
	downBlackLineRend->SetImageHeight(Window::height);
	upBlackLineRend->SetImageWidth(Window::width);
	downBlackLineRend->SetImageWidth(Window::width);

	upBlackLine->SetWorldPosition(DirectX::XMVectorSet(0,Window::height * 2,0,0));
	downBlackLine->SetWorldPosition(DirectX::XMVectorSet(0,Window::height * -2,0,0));

	radPE = systemManager->GetComponentList<PERadicalBlur>().back();
	radPE->SetSample(16);
	radPE->flag = false;
}

void GameEndScript::OnUpdate()
{
	if (gameM->CheckIsAllEmptyEachSide() == GameManagerScript::NOT_YET)
		return;

	radPE->flag = true;

	if(radTw == nullptr)
		radTw = tweenSystem->DoTween(0.0f, 1.0f, EasingType::EaseOutSine, 1.4f);
	else
	{
		camera->SetFov(std::lerp(DirectX::XMConvertToRadians(70.0f), DirectX::XMConvertToRadians(40.0f), radTw->GetValue()));

		radPE->SetBlurStrength(radTw->GetValue() * 0.8);
		upBlackLine->SetWorldPosition(DirectX::XMVectorLerp(DirectX::XMVectorSet(0, Window::height, 0, 0), DirectX::XMVectorSet(0, Window::height - 100, 0, 0), radTw->GetValue()));
		downBlackLine->SetWorldPosition(DirectX::XMVectorLerp(DirectX::XMVectorSet(0, -Window::height, 0, 0), DirectX::XMVectorSet(0, -Window::height + 100, 0, 0), radTw->GetValue()));
		GameFinishedRend->color = DirectX::XMVectorLerp(DirectX::XMVectorSet(1, 1, 1, 0), DirectX::XMVectorSplatOne(), radTw->GetValue());

		GameFinished->SetWorldRotation(DirectX::XMVectorSet(0, 0, 360 * 5 * radTw->GetValue(), 0));
		GameFinished->SetWorldScale(DirectX::XMVectorSet(1.1 - 0.4 * radTw->GetValue(), 1.1 - 0.4 * radTw->GetValue(), 1.1 - 0.4 * radTw->GetValue(), 0));
	}
}

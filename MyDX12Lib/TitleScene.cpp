#include "PCH/pch.h"
#include "TitleScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"
#include "GameStartUIScript.h"
#include "FloorScript.h"
#include "UIRenderer.h"
#include "Camera.h"
#include "TitleSpotLightScript.h"
#include "TitleNoiseScript.h"
#include "TableAndChairScript.h"
#include "TitleCardScript.h"
#include "ReadyUIScript.h"
#include "TitleFrameScript.h"
#include "GameEndScript.h"
#include "GameStartingScript.h"

TitleScene::TitleScene(std::string name) : GameScene::GameScene(name)
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Load()
{
	CreateEntity<FloorScript>("Floor");
	CreateEntity<TitleNoiseScript>("NoisePEEnt");
	CreateEntity<GameStartUIScript>("TitleUI");
	CreateEntity<TableAndChairScript>("TableChair");
	CreateEntity<TitleCardScript>("TitleCard");
	CreateEntity<TitleCardScript>("TitleCard");

	auto start = CreateEntity<TitleFrameScript>("FrameStart");
	auto exit = CreateEntity<TitleFrameScript>("FrameExit");

	start->GetComponent<TitleFrameScript>()->flag = TitleFrameScript::FLAG::START;
	exit->GetComponent<TitleFrameScript>()->flag = TitleFrameScript::FLAG::EXIT;


	DXRenderSystem::GetInstance().fogFlag = true;
	DXRenderSystem::GetInstance().SetFogRange(20);
	DXRenderSystem::GetInstance().SetFogColor(DirectX::XMVectorSet(0.2f,0.2f, 0.2f,0));

	DXRenderSystem::GetInstance().SetAmbientColor(DirectX::XMFLOAT3(0.03f,0.03f,0.03f));

	CreateEntity<TitleSpotLightScript>("SpotLight");
}

#include "CardGameScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"
#include "FloorScript.h"
#include "SquareWipeScript.h"
#include "Camera.h"
#include "DeckScript.h"
#include "HandScript.h"
#include "GameManagerScript.h"
#include "CardTextureScript.h"
#include "TableDeckScript.h"
#include "CPUScript.h"
#include "PlaceCardUIScript.h"
#include "GameEndScript.h"
#include "GameStartingScript.h"


CardGameScene::CardGameScene(std::string name) : GameScene::GameScene(name)
{
}

CardGameScene::~CardGameScene()
{
}

void CardGameScene::Load()
{
	CreateEntity<CardTextureScript>("CardTex");
	floor = CreateEntity<FloorScript>("Floor");
	CreateEntity<SquareWipeScript>("SquareWipe");
	auto td1 = CreateEntity<TableDeckScript>("TableDeck1");
	auto td2 = CreateEntity<TableDeckScript>("TableDeck2");
	CreateEntity<PlaceCardUIScript>("PlaceCardUI");

	td2->SetWorldPosition(DirectX::XMVectorSet(1.5,0.7,0,0));
	td1->SetWorldPosition(DirectX::XMVectorSet(-1.5, 0.7,0,0));

	auto pDeck = CreateEntity<DeckScript>("PlayerDeck");
	auto pHand = CreateEntity<HandScript>("PlayerHand");
	auto cDeck = CreateEntity<DeckScript>("CPUDeck");
	auto cHand = CreateEntity<HandScript>("CPUHand");

	auto pDeckSc = pDeck->GetComponent<DeckScript>();
	auto pHandSc = pHand->GetComponent<HandScript>();
	auto cDeckSc = cDeck->GetComponent<DeckScript>();
	auto cHandSc = cHand->GetComponent<HandScript>();

	pDeckSc->Init(DeckScript::PLAYER);
	pHandSc->Init(DeckScript::PLAYER);
	cDeckSc->Init(DeckScript::CPU);
	cHandSc->Init(DeckScript::CPU);

	CreateEntity<GameStartingScript>("GameStarting");
	CreateEntity<GameManagerScript>("GameManager");
	CreateEntity<CPUScript>("CPU");
	CreateEntity<GameEndScript>("GameEnd");

	DXRenderSystem::GetInstance().SetAmbientColor(DirectX::XMFLOAT3(0.4f, 0.4f, 0.4f));

	Camera::GetInstance().SetCameraRotation(DirectX::XMVectorSet(89,0,0,0));
	Camera::GetInstance().SetCameraPosition(DirectX::XMVectorSet(0,6,0,0));

	DLObj = CreateEntity<DirectionalLight>("DLOBJ");

	dlComp = DLObj->GetComponent<DirectionalLight>();

	dlComp->SetRotation(DirectX::XMVectorSet(70,19,0,0));

	dlComp->SetIsCastShadow(true);

	DXRenderSystem::GetInstance().SetCascadeShadowFarZ(15);
	DXRenderSystem::GetInstance().SetCascadeShadowCount(1);
}

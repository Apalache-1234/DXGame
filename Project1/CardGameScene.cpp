#include "CardGameScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"
#include "FloorScript.h"
#include "Camera.h"
#include "DirectionalLight.h"

CardGameScene::CardGameScene(std::string name) : GameScene::GameScene(name)
{
}

CardGameScene::~CardGameScene()
{
}

std::shared_ptr<Entity> floor1;
std::shared_ptr<Entity> DLObj;

std::shared_ptr<DirectionalLight> dlComp;

void CardGameScene::Load()
{
	floor1 = CreateEntity<SkinnedRenderer>("Floor");
	floor1->AddComponent<FloorScript>();

	SystemManager::GetInstance().SetAmbientColor(DirectX::XMFLOAT3(0.4f, 0.4f, 0.4f));

	Camera::GetInstance().SetCameraRotation(DirectX::XMVectorSet(89,0,0,0));

	DLObj = CreateEntity<DirectionalLight>("DLOBJ");

	dlComp = DLObj->GetComponent<DirectionalLight>();

	dlComp->SetRotation(DirectX::XMVectorSet(70,19,0,0));

	dlComp->SetIsCastShadow(true);
}

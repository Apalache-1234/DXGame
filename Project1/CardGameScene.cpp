#include "CardGameScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"
#include "FloorScript.h"
#include "Camera.h"


CardGameScene::CardGameScene(std::string name) : GameScene::GameScene(name)
{
}

CardGameScene::~CardGameScene()
{
}

void CardGameScene::Load()
{
	floor = CreateEntity<FloorScript>("Floor");

	DXRenderSystem::GetInstance().SetAmbientColor(DirectX::XMFLOAT3(0.4f, 0.4f, 0.4f));

	Camera::GetInstance().SetCameraRotation(DirectX::XMVectorSet(89,0,0,0));

	DLObj = CreateEntity<DirectionalLight>("DLOBJ");

	dlComp = DLObj->GetComponent<DirectionalLight>();

	dlComp->SetRotation(DirectX::XMVectorSet(70,19,0,0));

	dlComp->SetIsCastShadow(true);
}

#include "CardGameScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"

CardGameScene::CardGameScene(std::string name) : GameScene::GameScene(name)
{
}

CardGameScene::~CardGameScene()
{
}

void CardGameScene::Load()
{
	CreateEntity<SkinnedRenderer>("Player789");
	CreateEntity<SkinnedRenderer>("Player478");
	CreateEntity<SkinnedRenderer>("Player8792");

}

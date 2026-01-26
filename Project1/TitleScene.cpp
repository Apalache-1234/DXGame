#include "TitleScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"
#include "GameStartUIScript.h"

TitleScene::TitleScene(std::string name) : GameScene::GameScene(name)
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Load()
{
	CreateEntity<GameStartUIScript>("GameStartUI");
}

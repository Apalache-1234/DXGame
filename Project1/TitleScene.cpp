#include "TitleScene.h"
#include "SkinnedRenderer.h"
#include "Collider.h"
#include "RigidBody.h"

TitleScene::TitleScene(std::string name)
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Load()
{
	std::cout << "fainfuanw" << std::endl;

	CreateEntity<SkinnedRenderer>("Player");
	CreateEntity<SkinnedRenderer>("Player4");
	CreateEntity<SkinnedRenderer>("Player2");

}

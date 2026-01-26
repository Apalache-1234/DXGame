#include "GameScene.h"
#include "TitleScene.h"

GameScene::GameScene(std::string name)
{
	sceneName = name;
}

GameScene::~GameScene()
{
}

void GameScene::Load()
{
}

void GameScene::Reset()
{
	for (auto& ent : entites)
	{
		ent->Destroy();
	}

	entites.clear();
}

void GameScene::SetName(std::string name)
{
	sceneName = name;
}

std::string GameScene::GetName()
{
	return sceneName;
}

std::vector<std::shared_ptr<Entity>> GameScene::GetEntities()
{
	return entites;
}


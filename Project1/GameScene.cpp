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
	for (auto& ent : entities)
	{
		ent->Destroy();
	}

	entities.clear();
}

void GameScene::SetName(std::string name)
{
	sceneName = name;
}

void GameScene::Update()
{
	for (int i = 0;i < entities.size();i++)
	{
		if (entities[i]->IsDestoroyed())
		{
			entities.erase(entities.begin() + i);

		}
	}
}

std::string GameScene::GetName()
{
	return sceneName;
}

std::vector<std::shared_ptr<Entity>> GameScene::GetEntities()
{
	return entities;
}


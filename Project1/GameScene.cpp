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
	// std::remove_if を使って「削除対象」を末尾に集める
	auto it = std::remove_if(entities.begin(), entities.end(),
		[this](const std::shared_ptr<Entity>& e) {
			if (e->IsDestoroyed()) {
				return true; // vectorから削除対象にする
			}
			return false;
		});

	// 実際に vector から削除
	entities.erase(it, entities.end());
}

std::string GameScene::GetName()
{
	return sceneName;
}

std::vector<std::shared_ptr<Entity>> GameScene::GetEntities()
{
	return entities;
}


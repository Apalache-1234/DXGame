#include "GameScene.h"

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
}

void GameScene::SetName(std::string name)
{
	sceneName = name;
}

std::string GameScene::GetName()
{
	return sceneName;
}

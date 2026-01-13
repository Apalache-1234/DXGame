#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <windows.h>
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include "GameScene.h"

class SceneManager
{
public:
	SceneManager();

	~SceneManager();

	static SceneManager& GetInstance();

	void AddScene(GameScene scene);
	std::shared_ptr<GameScene> GetScene(std::string name);
	std::shared_ptr<GameScene> GetScene(UINT ind);
private:
	static std::unique_ptr<SceneManager> instance;

	std::vector<std::shared_ptr<GameScene>> scenes;
};
#endif
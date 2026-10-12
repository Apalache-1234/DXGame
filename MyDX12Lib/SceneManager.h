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

	std::shared_ptr<GameScene> GetCurrentGameScene();

	void ChangeGameScene(int ind);

	void ChangeGameScene(std::string name);

	void Update();

	void SaveEntitiesJson(const std::string scenePath);
	void LoadEntitiesJson(const std::string scenePath);

	void SaveScene();

	void LoadScene();

	void AddScene(std::shared_ptr<GameScene> scene);
	std::shared_ptr<GameScene> GetScene(std::string name);
	std::shared_ptr<GameScene> GetScene(UINT ind);
	std::vector<std::shared_ptr<GameScene>> GetScenes();
private:
	void ResetGameScene(std::shared_ptr<GameScene> scene);

	static std::unique_ptr<SceneManager> instance;

	std::vector<std::shared_ptr<GameScene>> scenes;
	std::unordered_map<std::string , std::shared_ptr<GameScene>> scenesN;

	std::shared_ptr<GameScene> currentGameScene;
};
#endif
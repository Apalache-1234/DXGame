#pragma once
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include "Component.h"
#include "SystemManager.h"

class Entity;

class GameScene
{
public:
	GameScene(std::string name);

	virtual ~GameScene();

	virtual void Load();

	void Reset();

	void SetName(std::string name);

	std::string GetName();

	std::vector<std::shared_ptr<Entity>> GetEntities();

	template <typename T, std::enable_if_t<std::is_base_of_v<Component, T>>* = nullptr>
	void CreateEntity(std::string name)
	{
		auto ent = SystemManager::GetInstance().CreateEntity<T>(name);

		entites.emplace_back(ent);
	}
private:
	std::vector<std::shared_ptr<Entity>> entites;

	std::string sceneName = {};
};
#pragma once
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>

class GameScene
{
public:
	GameScene();

	~GameScene();

	void SetName(std::string name);
	std::string GetName();
private:
	std::string sceneName = {};
};
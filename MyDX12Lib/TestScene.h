#pragma once
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include "GameScene.h"

class TestScene : public GameScene
{
public:
	TestScene(std::string name);

	~TestScene();

	void Load() override;
private:
};
#pragma once
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include "GameScene.h"

class CardGameScene : public GameScene
{
public:
	CardGameScene(std::string name);

	~CardGameScene();

	void Load() override;
};
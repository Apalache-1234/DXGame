#pragma once
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include "GameScene.h"

class TitleScene : public GameScene
{
public:
	TitleScene(std::string name);

	~TitleScene();

	void Load() override;
};
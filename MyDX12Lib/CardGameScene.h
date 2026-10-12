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
private:
	std::shared_ptr<Entity> floor;
	std::shared_ptr<Entity> DLObj;

	std::shared_ptr<DirectionalLight> dlComp;
};
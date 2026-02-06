#ifndef GAME_STARTING_SCRIPT_H
#define GAME_STARTING_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;
class UIRenderer;
class PEEdgeFilter;

class GameStartingScript : public ScriptComponent
{
public:
	GameStartingScript(std::shared_ptr<Entity> ent);

	~GameStartingScript();

	void OnStart() override;

	void StartAnim(std::function<void()> f);

	void OnUpdate() override;
protected:
private:
	bool animStarted = false;

	std::shared_ptr<PEEdgeFilter> edgePE;

	std::shared_ptr<UIRenderer> uiRend;

	std::shared_ptr<Entity> readyEnt;
	std::shared_ptr<UIRenderer> readyRend;

	shared_ptr<Tween<int>> spriteTw;

	float time;
	float interval = 0.02;

	std::vector<std::string> file_names;

	std::vector<ComPtr<ID3D12Resource>> sprites;

	Material mat;
};

#endif
#ifndef GAME_END_SCRIPT_H
#define GAME_END_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;
class PERadicalBlur;
class GameManagerScript;
class UIRenderer;

class GameEndScript : public ScriptComponent
{
public:
	GameEndScript(std::shared_ptr<Entity> ent);

	~GameEndScript();

	void OnStart() override;

	void OnUpdate() override;
protected:
private:
	std::shared_ptr<PERadicalBlur> radPE;

	std::shared_ptr<Tween<float>> radTw;

	std::shared_ptr<Entity> upBlackLine;
	std::shared_ptr<UIRenderer> upBlackLineRend;
	std::shared_ptr<Entity> downBlackLine;
	std::shared_ptr<UIRenderer> downBlackLineRend;

	std::shared_ptr<Entity> GameFinished;
	std::shared_ptr<UIRenderer> GameFinishedRend;

	Material mat;

	std::shared_ptr<GameManagerScript> gameM;
};

#endif
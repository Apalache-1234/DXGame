#ifndef GAME_START_UI_SCRIPT_H
#define GAME_START_UI_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PERadicalBlur.h"

class Entity;

class GameStartUIScript : public ScriptComponent
{
public:
	GameStartUIScript(std::shared_ptr<Entity> ent);

	~GameStartUIScript();

	void OnStart() override;

	void OnBlack();

	void OnUpdate() override;
protected:
private:
	std::shared_ptr<Tween < float>> tw;
	std::shared_ptr<Tween < XMVECTOR>> tw2;

	std::shared_ptr<PERadicalBlur> mF;

	std::vector<shared_ptr<Entity>> speedUIs;
	shared_ptr<Entity> BlackScreen;
};

#endif
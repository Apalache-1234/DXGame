#ifndef GAME_START_UI_SCRIPT_H
#define GAME_START_UI_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"

class Entity;

class GameStartUIScript : public ScriptComponent
{
public:
	GameStartUIScript(std::shared_ptr<Entity> ent);

	~GameStartUIScript();

	void OnStart() override;
protected:
};

#endif
#ifndef TABLE_AND_CHAIR_SCRIPT_H
#define TABLE_AND_CHAIR_SCRIPT_H


#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PENoiseFilter.h"

class Entity;

class TableAndChairScript : public ScriptComponent
{
public:
	TableAndChairScript(std::shared_ptr<Entity> ent);

	~TableAndChairScript();

	void OnStart() override;

	void OnUpdate() override;

private:
	std::shared_ptr<SkinnedRenderer> rend;


};

#endif
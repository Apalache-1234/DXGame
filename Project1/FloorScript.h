#ifndef FLOOR_SCRIPT_H
#define FLOOR_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;

class FloorScript : public ScriptComponent
{
public:
	FloorScript(std::shared_ptr<Entity> ent);

	~FloorScript();

	void OnStart() override;

	void OnUpdate() override;
protected:
private:
	std::shared_ptr<SkinnedRenderer> sRend;

	Material mat;
};

#endif
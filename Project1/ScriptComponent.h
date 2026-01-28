#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include <memory>
#include <typeindex>
#include <iostream>
#include "Component.h"
#include "TweenSystem.h"

class Entity;

class ScriptComponent : public Component
{
public:
	ScriptComponent(std::shared_ptr<Entity> ent);

	virtual ~ScriptComponent();

	virtual void OnMousePointerIn();

	virtual void OnMousePointerOn();

	virtual void OnMousePointerClick();

	virtual void OnMousePointerOut();

	virtual void OnStart();

	virtual void OnUpdate();

	virtual void OnFixedUpdate();

	virtual void OnDestroy();

	void Init() override;

protected:
	TweenSystem* tweenSystem;

	SystemManager* systemManager;
};

#endif
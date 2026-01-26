#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include <memory>
#include <typeindex>
#include "Component.h"

class Entity;

class ScriptComponent : public Component
{
public:
	ScriptComponent(std::shared_ptr<Entity> ent);

	~ScriptComponent();

	virtual void OnMousePointerIn();

	virtual void OnMousePointerOn();

	virtual void OnMousePointerClick();

	virtual void OnMousePointerOut();

	virtual void OnStart();

	virtual void OnUpdate();

	virtual void OnFixedUpdate();

	virtual void OnDestroy();

	std::shared_ptr<Entity> attachFrom;
protected:
	//このコンポーネントを持つエンティティーのポインタ

};

#endif
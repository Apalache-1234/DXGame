#ifndef COMPONENT_H
#define COMPONENT_H

#include <memory>
#include <typeindex>

class Entity;

class Component
{
public:
	Component(std::shared_ptr<Entity> ent);

	virtual ~Component();

	virtual void Init();

	std::shared_ptr<Entity> attachFrom;

	virtual std::type_index GetBaseType();
protected:
	//このコンポーネントを持つエンティティーのポインタ

};

#endif
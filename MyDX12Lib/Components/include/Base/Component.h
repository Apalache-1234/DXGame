#ifndef COMPONENT_H
#define COMPONENT_H

#include "ComponentList.h"

class Entity;

class Component : public std::enable_shared_from_this<Component>
{
public:
	Component(std::shared_ptr<Entity> ent);

	virtual ~Component();

	virtual void Init();

	std::weak_ptr<Entity> attachFrom;

	virtual std::string GetComponentName();

	void SetHash(UINT32 hashV);
	void SetIndex();

	UINT32 GetHash() noexcept;
	UINT32 GetTreeIndex() noexcept;
	UINT32 GetLeafIndex() noexcept;

	template<typename T>
	std::shared_ptr<T> Cast()
	{
		bool casted = ComponentList::IsParentComponent<T>(GetTreeIndex(), GetLeafIndex());

		if (casted)
			std::static_pointer_cast<T>(shared_from_this());

		return nullptr;
	}
protected:
	//このコンポーネントを持つエンティティーのポインタ
private:
	bool isSetHash = false;
	bool isSetIndex = false;
	UINT32 hash = 0;
	UINT32 treeIndex = 0;
	UINT32 leafIndex = 0;
};

#endif
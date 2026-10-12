#include "PCH/pch.h"
#include "Component.h"
#include "ComponentList.h"

Component::Component(std::shared_ptr<Entity> ent)
{
	attachFrom = ent;
}

Component::~Component()
{
	attachFrom.reset();
}

void Component::Init() {

}

std::string Component::GetComponentName()
{
	auto& typeID = typeid(*this);
	std::string className = typeID.name();

	auto compName = className.substr(6);

	return compName;
}

void Component::SetHash(UINT32 hashV)
{
	if (isSetHash)
		return;

	hash = hashV;
	isSetHash = true;
}

void Component::SetIndex()
{
	if (isSetIndex)
		return;

    int depth = 0;

    for (int i = IM_ARRAYSIZE(ComponentList::tree) - 1;0 <= i; i--)
    {
        depth = i;

        for (int j = 0;j < ComponentList::tree[depth].size(); j++)
        {
            const auto& leaf = ComponentList::tree[depth][j];
            if (leaf.hash == GetHash())
            {
				treeIndex = depth;
				leafIndex = j;
                break;
            }

        }
    }
	isSetIndex = true;
}

UINT32 Component::GetHash() noexcept
{
	return hash;
}

UINT32 Component::GetTreeIndex() noexcept
{
	return treeIndex;
}

UINT32 Component::GetLeafIndex() noexcept
{
	return leafIndex;
}

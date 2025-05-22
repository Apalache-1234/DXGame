#include "Object.h"
#include "Component.h"
#include "GameManager.h"

Object::Object(const std::string& name)
{
	this->name = name;
}

Object::~Object()
{
}

void Object::AddChild(std::shared_ptr<Object> obj)
{
	if (obj.get() == nullptr)
	{
		std::cerr << "追加する子供のポインタがnullptrです" << std::endl;

		return;
	}

	obj->SetParent(shared_from_this());
	
	objsChild.emplace_back(obj);
}

std::vector<std::shared_ptr<Object>> Object::GetChild()
{
	return objsChild;
}

std::shared_ptr<Object> Object::GetChild(std::string name)
{
	if (name.length() == 0)
	{
		std::cerr << "文字数が0" << std::endl;

		return nullptr;
	}

	for (auto obj : objsChild)
	{
		if (obj->name == name)
		{
			return obj;
		}
	}

	std::cerr << "指定された名前のオブジェクトが子供に存在しない" << std::endl;

	return nullptr;
}

std::shared_ptr<Object> Object::GetChild(int index)
{
	if (objsChild.size() <= index || index < 0)
	{
		std::cerr << "子供オブジェクトへの配列のインデックスが範囲外を超えている" << std::endl;

		return nullptr;
	}

	return objsChild[index];
}

void Object::SetParent(std::shared_ptr<Object> obj)
{
	if (obj.get() == nullptr)
	{
		std::cerr << "セットする親のポインタがnullptrです" << std::endl;

		return;
	}

	parent = obj;
}

void Object::SetName(std::string name)
{
	if (GameManager::GetInstance().isExitNameEntity(name))
	{
		int i = 1;

		while (true)
		{
			std::string nameNum = name + std::to_string(i);

			if (!GameManager::GetInstance().isExitNameEntity(nameNum))
			{
				name = nameNum;
				break;
			}

			i++;
		}
	}

	this->name = name;
}

std::string Object::GetName()
{
	return this->name;
}

std::shared_ptr<Object> Object::GetParent()
{
	if (parent == nullptr)
	{
		std::cout << "親が存在しません" << std::endl;

		return nullptr;
	}

	return parent;
}

bool Object::HasParent()
{
	bool hasParent = parent != nullptr;

	return hasParent;
}

bool Object::HasChild()
{
	return objsChild.size() > 0;
}

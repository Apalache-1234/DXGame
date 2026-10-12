#include "PCH/pch.h"
#include "Object.h"
#include "Component.h"
#include "SystemManager.h"

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

int Object::GetAsChildIndex()
{
	return asChildIndex;
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

	asChildIndex = parent.lock()->GetChild().size();
}

void Object::ResetParent()
{
	if (HasParent())
	{
		parent.lock()->ResetChild(this->GetName());

		parent.lock() = nullptr;
	}
}

void Object::ResetAllChild()
{
	for (int i = 0;i < objsChild.size();i++)
	{
		objsChild[i]->ResetParent();
	}

	objsChild.clear();
}

void Object::ResetChild(int num)
{
	if (num >= objsChild.size())
	{
		std::cerr << "その子のインデックスが存在しないため子のリセットの失敗" << std::endl;

		return;
	}

	objsChild[num]->parent.lock() = nullptr;

	objsChild.erase(objsChild.begin() +  num);
}

void Object::ResetChild(std::string name)
{
	for (int i = 0;i < objsChild.size() ;i++)
	{
		if (objsChild[i]->GetName() == name)
		{
			objsChild[i]->parent.lock() = nullptr;

			objsChild.erase(objsChild.begin() + i);

			return;
		}
	}

	std::cerr << "その子の名前が存在しないため子のリセットの失敗" << std::endl;

	return;
}

void Object::SetName(std::string name)
{
	if (SystemManager::GetInstance().isExitNameEntity(name))
	{
		int i = 1;

		while (true)
		{
			std::string nameNum = name + std::to_string(i);

			if (!SystemManager::GetInstance().isExitNameEntity(nameNum))
			{
				name = nameNum;
				break;
			}

			i++;
		}
	}

	this->name = name;
}

bool Object::IsDestoroyed()
{
	return isDestroyed;
}

void Object::Destroy()
{
	ResetParent();

	ResetAllChild();

	isDestroyed = true;
}

std::string Object::GetName()
{
	return this->name;
}

std::shared_ptr<Object> Object::GetParent()
{
	if (parent.lock() == nullptr)
	{
		std::cout << "親が存在しません" << std::endl;

		return nullptr;
	}

	return parent.lock();
}

bool Object::HasParent()
{
	bool hasParent = parent.lock() != nullptr;

	return hasParent;
}

bool Object::HasChild()
{
	return objsChild.size() > 0;
}

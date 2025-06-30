#ifndef LIGHT_H
#define LIGHT_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "MeshLoader.h"
#include "Utility.h"

class Entity;

class Light : public Component
{
public:
	Light(std::shared_ptr<Entity> ent);

	~Light();

	float GetPower();

	DirectX::XMVECTOR GetColor();

	void SetPower(float pow);

	void SetColor(DirectX::XMVECTOR col);
private:
	float power = 1;

	DirectX::XMVECTOR color = DirectX::XMVectorSet(0.3, 0, 0, 1);
};

#endif
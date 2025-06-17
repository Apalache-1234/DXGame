#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

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

class DirectionalLight : public Component
{
public:
	DirectionalLight(std::shared_ptr<Entity> ent);

	~DirectionalLight();

	DirectX::XMVECTOR GetLightDir();

	float GetReflection();

	DirectX::XMVECTOR GetColor();

	void SetReflection(float ref);

	void SetColor(DirectX::XMVECTOR col);
private:
	DirectX::XMVECTOR lightDir = Utility::forward;

	float reflection = 1;

	DirectX::XMVECTOR color = DirectX::XMVectorSet(0.3 , 0 , 0 , 1);
};

#endif
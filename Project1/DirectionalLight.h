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
#include "Light.h"

class Entity;

class DirectionalLight : public Light
{
public:
	DirectionalLight(std::shared_ptr<Entity> ent);

	~DirectionalLight();

	DirectX::XMVECTOR GetLightDir();

	DirectX::XMVECTOR GetRotation();

	void SetRotation(DirectX::XMVECTOR rot);

	std::type_index GetBaseType() override;
private:
	DirectX::XMVECTOR lightDir = Utility::forward;

	DirectX::XMVECTOR rotation = DirectX::XMVectorZero();


};

#endif
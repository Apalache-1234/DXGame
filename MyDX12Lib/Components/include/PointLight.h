#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

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

class PointLight : public Light
{
public:
	PointLight(std::shared_ptr<Entity> ent);

	~PointLight();

	float GetRadius();

	DirectX::XMVECTOR GetPosition();

	void SetRadius(float rad);

	DirectX::XMVECTOR position = DirectX::XMVectorZero();
private:
	float radius = 1;
};

#endif
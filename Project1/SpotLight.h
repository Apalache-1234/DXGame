#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "MeshLoader.h"
#include "Utility.h"
#include "Light.h"

class Entity;

class SpotLight : public Light
{
public:
	SpotLight(std::shared_ptr<Entity> ent);

	~SpotLight();

	float GetAngular();

	float GetLength();

	DirectX::XMVECTOR GetPosition();

	DirectX::XMVECTOR GetRotation();

	DirectX::XMVECTOR GetLightDir();

	void SetAngular(float rad);

	void SetLength(float len);

	void SetRotation(DirectX::XMVECTOR rot);

	DirectX::XMVECTOR position = DirectX::XMVectorZero();

	std::type_index GetBaseType() override;

	XMMATRIX lightVP;
private:
	float length = 13;
	float angular  = DirectX::XMConvertToRadians(45);
	DirectX::XMVECTOR lightDir = Utility::forward;
	DirectX::XMVECTOR rotation = DirectX::XMVectorZero();
};

#endif
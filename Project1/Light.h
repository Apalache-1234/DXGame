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

	std::type_index GetBaseType() override;

	void SetIsCastShadow(bool val);

	bool GetIsCastShadow();

	void SetShadowMapIndex(UINT ind);

	UINT GetShadowMapIndex();
private:
	float power = 0.5f;

	UINT shadowMapIndex = 0;

	bool isCastShadow = false;

	DirectX::XMVECTOR color = DirectX::XMVectorSet(0.6, 0.5, 0.5, 1);
};

#endif
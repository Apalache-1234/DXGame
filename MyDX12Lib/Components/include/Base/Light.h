#ifndef LIGHT_H
#define LIGHT_H

#include <memory>
#include <DirectXMath.h>
#include "Component.h"

class Entity;

class Light : public Component
{
public:
	~Light();

	float GetPower();

	DirectX::XMVECTOR GetColor();

	void SetPower(float pow);

	void SetColor(DirectX::XMVECTOR col);

	void SetIsCastShadow(bool val);

	bool GetIsCastShadow();

	void SetShadowMapIndex(UINT ind);

	UINT GetShadowMapIndex();
protected:
	Light(std::shared_ptr<Entity> ent);
private:

	float power = 0.5f;

	UINT shadowMapIndex = 0;

	bool isCastShadow = false;

	DirectX::XMVECTOR color = DirectX::XMVectorSet(0.6, 0.5, 0.5, 1);
};

#endif
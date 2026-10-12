#include "PCH/pch.h"
#include "SpotLight.h"
#include "Utility.h"

SpotLight::SpotLight(std::shared_ptr<Entity> ent) : Light::Light(ent)
{
}

SpotLight::~SpotLight()
{
}

float SpotLight::GetAngular()
{
	return angular;
}

float SpotLight::GetLength()
{
	return length;
}

DirectX::XMVECTOR SpotLight::GetPosition()
{
	return DirectX::XMVectorAdd(position, attachFrom.lock()->GetWorldPosition());
}

DirectX::XMVECTOR SpotLight::GetRotation()
{
	return rotation;
}

DirectX::XMVECTOR SpotLight::GetLightDir()
{
	auto local = rotation;

	auto x = DirectX::XMConvertToRadians(DirectX::XMVectorGetX(local));
	auto y = DirectX::XMConvertToRadians(DirectX::XMVectorGetY(local));
	auto z = DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(local));

	local = DirectX::XMVectorSet(x, y, z, 0);

	local = DirectX::XMQuaternionRotationRollPitchYawFromVector(local);

	auto result = DirectX::XMQuaternionMultiply(local, attachFrom.lock()->GetRotationAsQuaternion());

	return DirectX::XMVector3Rotate(lightDir, result);
}


void SpotLight::SetAngular(float rad)
{
	rad = std::clamp(rad , 0.0f , Utility::PI);

	angular = rad;
}

void SpotLight::SetLength(float len)
{
	len = std::fmax(len, 0.01f);

	length = len;
}

void SpotLight::SetRotation(DirectX::XMVECTOR rot)
{
	rotation = rot;
}

#include "PCH/pch.h"
#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(std::shared_ptr<Entity> ent) : Light::Light(ent)
{
}

DirectionalLight::~DirectionalLight()
{
}

DirectX::XMVECTOR DirectionalLight::GetLightDir()
{
    auto local = rotation;

    auto x = DirectX::XMConvertToRadians(DirectX::XMVectorGetX(local));
    auto y = DirectX::XMConvertToRadians(DirectX::XMVectorGetY(local));
    auto z = DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(local));

    local = DirectX::XMVectorSet(x,y,z,0);

    local = DirectX::XMQuaternionRotationRollPitchYawFromVector(local);
    
    auto result = DirectX::XMQuaternionMultiply(local , attachFrom.lock()->GetRotationAsQuaternion());

    return DirectX::XMVector3Rotate(lightDir , result);
}


DirectX::XMVECTOR DirectionalLight::GetRotation()
{
    return rotation;
}

void DirectionalLight::SetRotation(DirectX::XMVECTOR rot)
{
    rotation = rot;
}

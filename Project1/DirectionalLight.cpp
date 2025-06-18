#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(std::shared_ptr<Entity> ent) : Component::Component(ent)
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
    
    auto result = DirectX::XMQuaternionMultiply(local , attachFrom->GetRotationAsQuaternion());

    return DirectX::XMVector3Rotate(lightDir , result);
}

float DirectionalLight::GetReflection()
{
    return reflection;
}

DirectX::XMVECTOR DirectionalLight::GetColor()
{
    return color;
}

DirectX::XMVECTOR DirectionalLight::GetRotation()
{
    return rotation;
}

void DirectionalLight::SetReflection(float ref)
{
    reflection = std::max(ref , 0.0f);
}

void DirectionalLight::SetColor(DirectX::XMVECTOR col)
{
    color = DirectX::XMVectorClamp(col , DirectX::XMVectorZero() , DirectX::XMVectorSplatOne());
}

void DirectionalLight::SetRotation(DirectX::XMVECTOR rot)
{
    rotation = rot;
}

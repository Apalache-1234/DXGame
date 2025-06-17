#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

DirectionalLight::~DirectionalLight()
{
}

DirectX::XMVECTOR DirectionalLight::GetLightDir()
{
    return DirectX::XMVector3Rotate(lightDir , attachFrom->GetRotationAsQuaternion());
}

float DirectionalLight::GetReflection()
{
    return reflection;
}

DirectX::XMVECTOR DirectionalLight::GetColor()
{
    return color;
}

void DirectionalLight::SetReflection(float ref)
{
    reflection = std::max(ref , 0.0f);
}

void DirectionalLight::SetColor(DirectX::XMVECTOR col)
{
    color = DirectX::XMVectorClamp(col , DirectX::XMVectorZero() , DirectX::XMVectorSplatOne());
}



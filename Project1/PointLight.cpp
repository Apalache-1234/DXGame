#include "PointLight.h"

PointLight::PointLight(std::shared_ptr<Entity> ent) : Light::Light(ent)
{
}

PointLight::~PointLight()
{
}

float PointLight::GetRadius()
{
    return radius;
}

DirectX::XMVECTOR PointLight::GetPosition()
{
    return DirectX::XMVectorAdd(position , attachFrom->GetWorldPosition());
}

void PointLight::SetRadius(float rad)
{
    radius = rad;
}

std::type_index PointLight::GetBaseType()
{
    return typeid(Light);
}

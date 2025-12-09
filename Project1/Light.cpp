#include "Light.h"

Light::Light(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

Light::~Light()
{
}

float Light::GetPower()
{
    return power;
}

DirectX::XMVECTOR Light::GetColor()
{
    return color;
}

void Light::SetPower(float pow)
{
    power = std::fmax(pow, 0.0f);
}

void Light::SetColor(DirectX::XMVECTOR col)
{
    color = DirectX::XMVectorClamp(col, DirectX::XMVectorZero(), DirectX::XMVectorSplatOne());
}

std::type_index Light::GetBaseType()
{
    return typeid(Component);
}

void Light::SetIsCastShadow(bool val)
{

}

bool Light::GetIsCastShadow()
{
    return false;
}

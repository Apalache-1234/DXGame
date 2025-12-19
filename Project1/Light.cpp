#include "Light.h"
#include "DXRenderSystem.h"

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
    isCastShadow = val;

    DXRenderSystem::GetInstance().UpdateLightCastingShadow();
}

bool Light::GetIsCastShadow()
{
    return isCastShadow;
}

void Light::SetShadowMapIndex(UINT ind)
{
    if (DXRenderSystem::GetInstance().GetIsShadowMapRendering())
    {
        shadowMapIndex = ind;
    }
}

UINT Light::GetShadowMapIndex()
{
    return shadowMapIndex;
}

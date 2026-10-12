#include "PCH/pch.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::SetCameraTransform(DirectX::XMVECTOR position, DirectX::XMVECTOR rotation)
{
}
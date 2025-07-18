#include "CameraComponent.h"

CameraComponent::CameraComponent(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::SetCameraTransform(DirectX::XMVECTOR position, DirectX::XMVECTOR rotation)
{
	SetCameraPosition(position);

	SetCameraRotation(rotation);
}

std::type_index CameraComponent::GetBaseType()
{
	return typeid(Component);
}

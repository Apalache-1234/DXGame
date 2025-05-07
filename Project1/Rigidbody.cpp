#include "Rigidbody.h"

Rigidbody::Rigidbody(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
}

Rigidbody::~Rigidbody()
{
}

DirectX::XMVECTOR Rigidbody::GetVelocity()
{
	return velocity;
}

void Rigidbody::SetVelocity(DirectX::XMVECTOR vel)
{
	vel = DirectX::XMVectorScale(vel , 1 / mass);

	velocity = vel;
}

void Rigidbody::AddVelocity(DirectX::XMVECTOR vel)
{
	vel = DirectX::XMVectorScale(vel, 1 / mass);

	velocity = DirectX::XMVectorAdd(vel, velocity);
}

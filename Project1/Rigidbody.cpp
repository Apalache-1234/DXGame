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
	velocity = vel;
}

void Rigidbody::AddForce(DirectX::XMVECTOR vel)
{
	vel = DirectX::XMVectorScale(vel, 1 / mass);

	velocity = DirectX::XMVectorAdd(vel, velocity);
}

float Rigidbody::GetMass()
{
	return mass;
}

void Rigidbody::SetMass(float val)
{
	if (val < Utility::ep)
		val = Utility::ep;
	
	mass = val;
}

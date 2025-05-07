#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <memory>
#include "Component.h"

class Rigidbody : public Component
{
public:
	Rigidbody(std::shared_ptr<Entity> ent);

	~Rigidbody();

	DirectX::XMVECTOR GetVelocity();

	void SetVelocity(DirectX::XMVECTOR vel);

	void AddVelocity(DirectX::XMVECTOR vel);

	float mass = 1;

	bool isStatic = false;

private:
	DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);
};

#endif



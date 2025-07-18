#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <memory>
#include "Component.h"
#include "utility.h" 

class Rigidbody : public Component
{
public:
	Rigidbody(std::shared_ptr<Entity> ent);

	~Rigidbody();

	DirectX::XMVECTOR GetVelocity();

	void SetVelocity(DirectX::XMVECTOR vel);

	void AddForce(DirectX::XMVECTOR vel);

	float GetMass();

	void SetMass(float val);

	bool isStatic = false;

	std::type_index GetBaseType() override;
private:
	DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);

	float mass = 1;
};

#endif



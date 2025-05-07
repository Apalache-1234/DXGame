#include "Collider.h"

Collider::Collider(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
	SetColliderType(Cube);
}

Collider::~Collider()
{
}

std::vector<DirectX::XMVECTOR> Collider::GetCurrentVerticesPos()
{
	std::vector<DirectX::XMVECTOR> cpy = verticesPos;

	for (auto& pos : cpy)
	{
		pos = DirectX::XMVectorMultiply(pos, scale);

		pos = DirectX::XMVectorMultiply(pos, attachFrom->GetScale());

		auto mat1 = DirectX::XMMatrixRotationQuaternion(attachFrom->GetWorldQuaternion());
		auto mat2 = DirectX::XMMatrixRotationQuaternion(attachFrom->GetLocalQuaternion());

		auto rotationMat = mat1 * mat2;

		pos = XMVector3Transform(pos, -rotationMat);

		pos = DirectX::XMVectorAdd(pos, position);

		pos = DirectX::XMVectorAdd(pos, attachFrom->GetWorldPosition());
	}

	return cpy;
}

void Collider::SetColliderType(ColliderType type)
{
	switch (currentType)
	{
	case Cube:
		verticesPos.emplace_back(DirectX::XMVectorSet(0.5, 0.5, 0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(-0.5, 0.5, 0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(0.5, 0.5, -0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(-0.5, 0.5, -0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(0.5, -0.5, 0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(-0.5, -0.5, 0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(0.5, -0.5, -0.5, 0));
		verticesPos.emplace_back(DirectX::XMVectorSet(-0.5, -0.5, -0.5, 0));
		break;
	}

	currentType = type;

}

Collider::ColliderType Collider::GetColliderType()
{
	return currentType;
}

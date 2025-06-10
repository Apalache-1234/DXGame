#include "Entity.h"
#include "Utility.h"
#include "SkinnedRenderer.h"

Entity::Entity(const std::string& name) : Object::Object(name)
{
	worldPosition = DirectX::XMVectorSet(0,0,0,0);

	localPosition = DirectX::XMVectorSet(0, 0, 0, 0);

	worldScale = DirectX::XMVectorSet(1, 1, 1, 0);

	localScale = DirectX::XMVectorSet(1, 1, 1, 0);

	worldRotation = DirectX::XMVectorSet(0, 0, 0, 0);

	localRotation = DirectX::XMVectorSet(0, 0, 0, 0);
}

Entity::~Entity()
{
	std::cout << GetName() << "‚ª”jŠü‚³‚ê‚½" << std::endl;
}

void Entity::Init()
{
	SetWorldPosition(DirectX::XMVectorZero());
	SetLocalPosition(DirectX::XMVectorZero());

	SetWorldScale(DirectX::XMVectorSplatOne());
	SetLocalScale(DirectX::XMVectorSplatOne());

	SetWorldRotation(DirectX::XMVectorZero());
	SetLocalRotation(DirectX::XMVectorZero());

	rotation = XMVectorZero();

	components.clear();

	AddComponent<SkinnedRenderer>();
}

DirectX::XMVECTOR Entity::GetWorldPosition()
{
	return worldPosition;
}

DirectX::XMVECTOR Entity::GetLocalPosition()
{
	if (HasParent())
		return localPosition;
	else
		return worldPosition;
}

DirectX::XMVECTOR Entity::GetWorldScale()
{
	return worldScale;
}

DirectX::XMVECTOR Entity::GetLocalScale()
{
	if (HasParent())
		return localScale;
	else
		return worldScale;
}

DirectX::XMVECTOR Entity::GetScale()
{
	auto scale = GetWorldScale();

	float scaleX = DirectX::XMVectorGetX(scale);
	float scaleY = DirectX::XMVectorGetY(scale);
	float scaleZ = DirectX::XMVectorGetZ(scale);

	if (HasParent())
	{
		scale = GetLocalScale();

		scaleX *= DirectX::XMVectorGetX(scale);
		scaleY *= DirectX::XMVectorGetY(scale);
		scaleZ *= DirectX::XMVectorGetZ(scale);
	}

	return DirectX::XMVectorSet(scaleX, scaleY, scaleZ, 0);
}

DirectX::XMVECTOR Entity::GetWorldRotation()
{
	return worldRotation;
}

DirectX::XMVECTOR Entity::GetWorldQuaternion()
{
	DirectX::XMVECTOR resultQ = DirectX::XMVectorSet(0, 0, 0, 0);

	auto rotation = GetWorldRotation();

	float rotX = XMConvertToRadians(DirectX::XMVectorGetX(rotation));
	float rotY = XMConvertToRadians(DirectX::XMVectorGetY(rotation));
	float rotZ = XMConvertToRadians(DirectX::XMVectorGetZ(rotation));

	auto XQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), rotX);
	auto YQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), rotY);
	auto ZQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 0, 1, 0), rotZ);

	resultQ = Utility::GetZYXMultiplyQuaternion(XQuaternion, YQuaternion, ZQuaternion);

	return resultQ;
}

DirectX::XMVECTOR Entity::GetLocalRotation()
{
	return localRotation;
}

DirectX::XMVECTOR Entity::GetLocalQuaternion()
{
	DirectX::XMVECTOR resultQ = DirectX::XMVectorSet(0, 0, 0, 0);

	auto rotation = GetLocalRotation();

	float rotX = XMConvertToRadians(DirectX::XMVectorGetX(rotation));
	float rotY = XMConvertToRadians(DirectX::XMVectorGetY(rotation));
	float rotZ = XMConvertToRadians(DirectX::XMVectorGetZ(rotation));

	auto XQuaternion = DirectX::XMQuaternionRotationAxis(GetRight(), rotX);
	auto YQuaternion = DirectX::XMQuaternionRotationAxis(GetUp(), rotY);
	auto ZQuaternion = DirectX::XMQuaternionRotationAxis(GetForward(), rotZ);

	resultQ = Utility::GetZYXMultiplyQuaternion(XQuaternion, YQuaternion, ZQuaternion);

	return resultQ;
}

DirectX::XMVECTOR Entity::GetRight()
{
	return right;
}

DirectX::XMVECTOR Entity::GetUp()
{
	return up;
}

DirectX::XMVECTOR Entity::GetForward()
{
	return forward;
}

void Entity::SetWorldPosition(DirectX::XMVECTOR position)
{
	worldPosition = position;

	DirectX::XMMATRIX localMat;

	DirectX::XMVECTOR transPosition;

	if (HasParent())
	{
    	std:shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(GetParent());

		if (ent != nullptr)
		{
			auto subVec = DirectX::XMVectorSubtract(worldPosition, ent->GetWorldPosition());

			localMat = Utility::CreateTransMat(ent->right, ent->up, ent->forward);

			transPosition = DirectX::XMVector3Transform(subVec, localMat);

			localPosition = transPosition;
		}
		else
			localPosition = worldPosition;
	}
	else
	{
		localPosition = worldPosition;
	}

		
	if (HasChild())
	{
		for (auto obj : GetChild())
		{
	    	std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(obj);


			if (ent != nullptr)
			{

				localMat = Utility::CreateTransMat(this->right, this->up, this->forward);

				transPosition = DirectX::XMVector3Transform(ent->GetLocalPosition(), XMMatrixInverse(nullptr, localMat));

				ent->SetWorldPosition(DirectX::XMVectorAdd(worldPosition, transPosition));
			}
		}
	}

}

DirectX::XMVECTOR QuaternionToEulerZYX(DirectX::XMVECTOR q)
{
	float qx = DirectX::XMVectorGetX(q);
	float qy = DirectX::XMVectorGetY(q);
	float qz = DirectX::XMVectorGetZ(q);
	float qw = DirectX::XMVectorGetW(q);

	float sinr_cosp = 2.0f * (qw * qx + qy * qz);
	float cosr_cosp = 1.0f - 2.0f * (qx * qx + qy * qy);
	float x = std::atan2(sinr_cosp, cosr_cosp); 

	float sinp = 2.0f * (qw * qy - qz * qx);
	float y;
	if (std::abs(sinp) >= 1)
		y = std::copysign(DirectX::XM_PI / 2.0f, sinp); 
	else
		y = std::asin(sinp);

	float siny_cosp = 2.0f * (qw * qz + qx * qy);
	float cosy_cosp = 1.0f - 2.0f * (qy * qy + qz * qz);
	float z = std::atan2(siny_cosp, cosy_cosp); 

	return DirectX::XMVectorSet(
		DirectX::XMConvertToDegrees(x), 
		DirectX::XMConvertToDegrees(y), 
		DirectX::XMConvertToDegrees(z), 
		0.0f
	);
}

void Entity::SetQuaternion(DirectX::XMVECTOR q)
{
	auto result = DirectX::XMQuaternionMultiply(GetRotationAsQuaternion(), q);

	rotation = QuaternionToEulerZYX(result);
}

DirectX::XMVECTOR Entity::GetRotation()
{
	return rotation;
}

DirectX::XMVECTOR Entity::GetRotationAsQuaternion()
{
	DirectX::XMVECTOR resultQ = DirectX::XMVectorSet(0, 0, 0, 0);

	float rotX = XMConvertToRadians(DirectX::XMVectorGetX(rotation));
	float rotY = XMConvertToRadians(DirectX::XMVectorGetY(rotation));
	float rotZ = XMConvertToRadians(DirectX::XMVectorGetZ(rotation));

	auto XQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), rotX);
	auto YQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), rotY);
	auto ZQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 0, 1, 0), rotZ);

	resultQ = Utility::GetZYXMultiplyQuaternion(XQuaternion, YQuaternion, ZQuaternion);

	return resultQ;
}

void Entity::SetLocalPosition(DirectX::XMVECTOR position)
{
	localPosition = position;

	DirectX::XMMATRIX localMat;

	DirectX::XMVECTOR transPosition;

	if (HasParent())
	{
	    std:shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(GetParent());

		if (ent != nullptr)
		{
			localMat = Utility::CreateTransMat(ent->right, ent->up, ent->forward);

			transPosition = DirectX::XMVector3Transform(GetLocalPosition(), XMMatrixInverse(nullptr, localMat));

			SetWorldPosition(DirectX::XMVectorAdd(ent->worldPosition, transPosition));
		}
	}
	else
	{
		SetWorldPosition(position);
	}

	if (HasChild())
	{
		for (auto obj : GetChild())
		{
			for (auto obj : GetChild())
			{
				std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(obj);

				if (ent != nullptr)
				{
					localMat = Utility::CreateTransMat(this->right, this->up, this->forward);

					transPosition = DirectX::XMVector3Transform(ent->GetLocalPosition(), XMMatrixInverse(nullptr, localMat));

					ent->SetWorldPosition(DirectX::XMVectorAdd(worldPosition, transPosition));
				}
			}
		}
	}
}

void Entity::SetWorldScale(DirectX::XMVECTOR scale)
{
	auto prevScale = worldScale;

	worldScale = scale;

	if (!HasParent())
		localScale = worldScale;
	else
	{
	    std:shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(GetParent());

		if (ent != nullptr)
			localScale = DirectX::XMVectorDivide(worldScale, ent->GetWorldScale());
		else
			localScale = worldScale;
	}

	if (HasChild())
	{
		for (auto obj : GetChild())
		{
			for (auto obj : GetChild())
			{
				std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(obj);

				DirectX::XMVECTOR offset;

				if (ent != nullptr)
				{
					auto subVec = ent->GetLocalPosition();

					auto magnitudeX = subVec.m128_f32[0] / prevScale.m128_f32[0];
					auto magnitudeY = subVec.m128_f32[1] / prevScale.m128_f32[1];
					auto magnitudeZ = subVec.m128_f32[2] / prevScale.m128_f32[2];

					offset = DirectX::XMVectorSet(magnitudeX * worldScale.m128_f32[0], magnitudeY * worldScale.m128_f32[1], magnitudeZ * worldScale.m128_f32[2], 0);

					if (DirectX::XMVector3IsNaN(offset))
						offset = DirectX::XMVectorZero();

					ent->SetLocalPosition(offset);

					auto subScale = DirectX::XMVectorAdd(scale, -prevScale);

					ent->SetWorldScale(DirectX::XMVectorAdd(ent->GetWorldScale() , subScale));
				}
			}
		}
	}
}

void Entity::SetLocalScale(DirectX::XMVECTOR scale)
{
	localScale = scale;
}

DirectX::XMVECTOR prevWorldRotation = DirectX::g_XMZero;

void Entity::SetWorldRotation(DirectX::XMVECTOR rotation)
{
	worldRotation = rotation;

	auto subRot = DirectX::XMVectorSubtract(worldRotation , prevWorldRotation);

	auto rotX = DirectX::XMConvertToRadians(DirectX::XMVectorGetX(subRot));
	auto rotY = DirectX::XMConvertToRadians(DirectX::XMVectorGetY(subRot));
	auto rotZ = DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(subRot));

	SetQuaternion(DirectX::XMQuaternionRotationAxis(Utility::right, rotX));
	SetQuaternion(DirectX::XMQuaternionRotationAxis(Utility::up, rotY));
	SetQuaternion(DirectX::XMQuaternionRotationAxis(Utility::forward, rotZ));

	if (HasChild())
	{
		for (auto obj : GetChild())
		{
			std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(obj);

			if (ent != nullptr)
			{

				ent->SetWorldRotation(GetWorldRotation());
			}
		}
	}

	AdjustDirectionVector();

	prevWorldRotation = worldRotation;
}

DirectX::XMVECTOR prevLocalRotation = DirectX::g_XMZero;

void Entity::SetLocalRotation(DirectX::XMVECTOR rotation)
{
	localRotation = rotation;

	auto subRot = DirectX::XMVectorSubtract(localRotation, prevLocalRotation);

	auto rotX = DirectX::XMConvertToRadians(DirectX::XMVectorGetX(subRot));
	auto rotY = DirectX::XMConvertToRadians(DirectX::XMVectorGetY(subRot));
	auto rotZ = DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(subRot));

	SetQuaternion(DirectX::XMQuaternionRotationAxis(right, rotX));
	SetQuaternion(DirectX::XMQuaternionRotationAxis(up, rotY));
	SetQuaternion(DirectX::XMQuaternionRotationAxis(forward, rotZ));

	if (HasChild())
	{
		for (auto obj : GetChild())
		{
			std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(obj);

			if (ent != nullptr)
			{
				ent->SetLocalRotation(GetLocalRotation());
			}
		}
	}

	AdjustDirectionVector();

	prevLocalRotation = localRotation;
}

void Entity::SetRotation(DirectX::XMVECTOR rot)
{
	rotation = rot;
}

void Entity::AddWorldPosition(DirectX::XMVECTOR position)
{
	auto pos = GetWorldPosition();

	auto result = DirectX::XMVectorAdd(pos, position);

	SetWorldPosition(result);
}

void Entity::AddLocalPosition(DirectX::XMVECTOR position)
{
	auto pos = GetLocalPosition();

	auto result = DirectX::XMVectorAdd(pos, position);

	SetLocalPosition(result);
}

void Entity::AddWorldRotation(DirectX::XMVECTOR rotation)
{
	auto rot = GetWorldRotation();

	auto result = DirectX::XMVectorAdd(rot, rotation);

	SetWorldRotation(result);
}

void Entity::AddLocalRotation(DirectX::XMVECTOR rotation)
{
	auto rot = GetLocalRotation();

	auto result = DirectX::XMVectorAdd(rot, rotation);

	SetLocalRotation(result);
}

void Entity::AddWorldScale(DirectX::XMVECTOR scale)
{
	auto sc = GetWorldScale();

	auto result = DirectX::XMVectorAdd(sc, scale);

	SetWorldScale(result);
}

void Entity::Destroy()
{
	Object::Destroy();

	components.clear();
}

void Entity::AddLocalScale(DirectX::XMVECTOR scale)
{
	auto sc = GetLocalScale();

	auto result = DirectX::XMVectorAdd(sc, scale);

	SetLocalScale(result);
}

void Entity::AdjustDirectionVector()
{
	XMVECTOR rotation = GetRotationAsQuaternion();

	right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotation);
	up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotation);
	forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotation);

	if (!HasChild())
		return;

	for (auto obj : GetChild())
	{
		std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(obj);

		if (ent != nullptr)
		{
			auto rightTemp = DirectX::XMVectorScale(right, DirectX::XMVectorGetX(ent->GetLocalPosition()));
			auto upTemp = DirectX::XMVectorScale(up, DirectX::XMVectorGetY(ent->GetLocalPosition()));
			auto forwardTemp = DirectX::XMVectorScale(forward, DirectX::XMVectorGetZ(ent->GetLocalPosition()));

			auto resultVec = DirectX::XMVectorAdd(rightTemp, upTemp);
			resultVec = DirectX::XMVectorAdd(resultVec, forwardTemp);

			ent->SetWorldPosition(DirectX::XMVectorAdd(GetWorldPosition(), resultVec));
		}
	}
}


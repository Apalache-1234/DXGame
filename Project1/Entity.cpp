#include "Entity.h"
#include "Utility.h"
#include "Renderer.h"

Entity::Entity(const std::string& name) : Object::Object(name)
{
	worldPosition = DirectX::XMVectorSet(0,0,0,0);

	localPosition = DirectX::XMVectorSet(0, 0, 0, 0);

	worldScale = DirectX::XMVectorSet(1, 1, 1, 0);

	localScale = DirectX::XMVectorSet(1, 1, 1, 0);

	worldRotation = DirectX::XMVectorSet(0, 0, 0, 0);

	localRotation = DirectX::XMVectorSet(0, 0, 0, 0);

	GameManager::GetInstance().SetEntity(std::shared_ptr<Entity>(this));
}

Entity::~Entity()
{
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

	scale = GetLocalScale();

	scaleX *= DirectX::XMVectorGetX(scale);
	scaleY *= DirectX::XMVectorGetY(scale);
	scaleZ *= DirectX::XMVectorGetZ(scale);

	return scale;
}

DirectX::XMVECTOR Entity::GetWorldRotation()
{
	return worldRotation;
}

DirectX::XMVECTOR Entity::GetWorldQuaternion()
{
	/*	auto XQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), rotX);
	auto YQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), rotY);
	auto ZQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 0, 1, 0), rotZ);

	resultQ = DirectX::XMQuaternionMultiply(XQuaternion, YQuaternion);
	resultQ = DirectX::XMQuaternionMultiply(resultQ, ZQuaternion);*/
	DirectX::XMVECTOR resultQ = DirectX::XMVectorSet(0, 0, 0, 0);

	auto rotation = GetWorldRotation();

	float rotX = XMConvertToRadians(DirectX::XMVectorGetX(rotation));
	float rotY = XMConvertToRadians(DirectX::XMVectorGetY(rotation));
	float rotZ = XMConvertToRadians(DirectX::XMVectorGetZ(rotation));

	

	return DirectX::XMQuaternionRotationRollPitchYaw(rotX, rotY, rotZ);
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

	resultQ = DirectX::XMQuaternionMultiply(XQuaternion, YQuaternion);
	resultQ = DirectX::XMQuaternionMultiply(resultQ, ZQuaternion);

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

void Entity::SetQuaternion(DirectX::XMVECTOR q)
{
	auto matrix = DirectX::XMMatrixRotationQuaternion(q);

	auto x =  atan(matrix.r[1].m128_f32[2]);
	auto y = atan2(matrix.r[2].m128_f32[0], matrix.r[0].m128_f32[0]);
	auto z = atan2(matrix.r[1].m128_f32[0], matrix.r[1].m128_f32[1]);

	x = DirectX::XMConvertToDegrees(x);
	y = DirectX::XMConvertToDegrees(y);
	z = DirectX::XMConvertToDegrees(z);

	SetWorldRotation(DirectX::XMVectorSet(x,y,z,0));
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

				if (ent != nullptr)
				{
					auto subVec = ent->GetLocalPosition();

					auto magnitudeX = subVec.m128_f32[0] / prevScale.m128_f32[0];
					auto magnitudeY = subVec.m128_f32[1] / prevScale.m128_f32[1];
					auto magnitudeZ = subVec.m128_f32[2] / prevScale.m128_f32[2];

					auto offset = DirectX::XMVectorSet(magnitudeX * worldScale.m128_f32[0], magnitudeY * worldScale.m128_f32[1], magnitudeZ * worldScale.m128_f32[2], 0);



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

void Entity::SetWorldRotation(DirectX::XMVECTOR rotation)
{
	worldRotation = rotation;

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
}

void Entity::SetLocalRotation(DirectX::XMVECTOR rotation)
{
	localRotation = rotation;

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

void Entity::AddLocalScale(DirectX::XMVECTOR scale)
{
	auto sc = GetLocalScale();

	auto result = DirectX::XMVectorAdd(sc, scale);

	SetLocalScale(result);
}

void Entity::AdjustDirectionVector()
{
	auto mat1 = DirectX::XMMatrixRotationQuaternion(GetWorldQuaternion());


	auto rotationMat = mat1;

	right = XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), rotationMat);
	up = XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), rotationMat);
	forward = XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), rotationMat);

	auto mat2 = DirectX::XMMatrixRotationQuaternion(GetLocalQuaternion());

	rotationMat = mat2;

	right = XMVector3TransformNormal(right, rotationMat);
	up = XMVector3TransformNormal(up, rotationMat);
	forward = XMVector3TransformNormal(forward, rotationMat);

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


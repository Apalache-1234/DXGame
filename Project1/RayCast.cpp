#include "RayCast.h"
#include "GameManager.h"
#include "Utility.h"

RayCast::RayCast(DirectX::XMVECTOR startPoint , DirectX::XMVECTOR dir, float distance)
{
	this->startPoint = startPoint;
	direction = dir;
	this->distance = distance;

	CheckHit();
}

RayCast::~RayCast()
{
}

bool RayCast::GetIsHit()
{
	return isHit;
}

void RayCast::ParseRayInteract(DirectX::XMVECTOR p1, DirectX::XMVECTOR p2, DirectX::XMVECTOR p3)
{
	auto edge1 = DirectX::XMVectorSubtract(p1, p2);
	auto edge2 = DirectX::XMVectorSubtract(p1, p3);

	auto cross = DirectX::XMVector3Cross(direction, edge2);
	auto detM = DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge1, cross));

	if (detM < Utility::ep)
	{
		isHit = false;

		return;
	}

	auto denom = 1 / detM;

	auto o = DirectX::XMVectorSubtract(startPoint, p1);

	auto u = denom * DirectX::XMVectorGetX( DirectX::XMVector3Dot(o, cross));

	if (u < 0 || u > 1)
	{
		isHit = false;

		return;
	}

	auto p = DirectX::XMVector3Cross(edge1, o);

	auto v = denom * DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, p));

	if (u + v < 0 || u + v > 1)
	{
		isHit = false;

		return;
	}

	auto t = denom * DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge2, p));

	if (t < distance)
	{
		isHit = false;

		return;
	}

	isHit = true;

	auto uVec = DirectX::XMVectorScale(edge1 , u);
	auto vVec = DirectX::XMVectorScale(edge2 , v);

	auto result = DirectX::XMVectorAdd(p1 , uVec);
	result = DirectX::XMVectorAdd(result , vVec);

	hitPoint = result;
}

void RayCast::CheckHit()
{
	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	for (int i = 0; i < colList.size(); i++)
	{
		auto pair = colList[i]->GetTriangles();

		for (int i = 0; i < pair.first->size(); i++)
		{
			auto vertices = *pair.second;

			auto p1 = DirectX::XMLoadFloat3(&vertices[i].vertex);
			auto p2 = DirectX::XMLoadFloat3(&vertices[i + 1].vertex);
			auto p3 = DirectX::XMLoadFloat3(&vertices[i + 2].vertex);

			ParseRayInteract(p1, p2, p3);
		}
	}
}

#include "RayCast.h"
#include "GameManager.h"
#include "Utility.h"
#include "DebugRayCast.h"

RayCast::RayCast(DirectX::XMVECTOR startPoint , DirectX::XMVECTOR dir, float distance)
{
	this->startPoint = startPoint;
	direction = dir;
	this->distance = distance;

	CheckHit();

	auto DebugRay = GameManager::GetInstance().GetComponentList<DebugRayCast>();

	for (int i = 0; i < DebugRay.size(); i++)
	{
		DebugRay[i]->AddRayCast(*this);
	}
}

RayCast::~RayCast()
{
}

std::vector<RayCast::Info> RayCast::GetInfo()
{
	return infos;
}

DirectX::XMVECTOR RayCast::GetStartPoint()
{
	return startPoint;
}

DirectX::XMVECTOR RayCast::GetDirection()
{
	return direction;
}

float RayCast::GetDistance()
{
	return distance;
}

bool RayCast::GetIsHit()
{
	return isHit;
}

void RayCast::ParseRayInteract(DirectX::XMVECTOR p1, DirectX::XMVECTOR p2, DirectX::XMVECTOR p3 , std::shared_ptr<Collider> col)
{
	auto edge1 = DirectX::XMVectorSubtract(p2, p1);

	auto edge2 = DirectX::XMVectorSubtract(p3, p1);

	auto cross = DirectX::XMVector3Cross(direction , edge2);

	auto detM = DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge1, cross));

	if (abs(detM) < Utility::ep)
	{
		return;
	}

	auto denom = 1 / detM;

	auto o = DirectX::XMVectorSubtract(startPoint, p1);

	auto u = denom * DirectX::XMVectorGetX( DirectX::XMVector3Dot(o, cross));

	if (u < 0 || u > 1)
	{
		return;
	}

	auto p = DirectX::XMVector3Cross(o , edge1);

	auto v = denom * DirectX::XMVectorGetX(DirectX::XMVector3Dot(direction, p));

	if (v < 0 || u + v < 0 || u + v > 1)
	{
		return;
	}

	auto t = denom * DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge2, p));

	if (t > distance)
	{
		return;
	}

	isHit = true;

	auto uVec = DirectX::XMVectorScale(edge1 , u);
	auto vVec = DirectX::XMVectorScale(edge2 , v);

	auto result = DirectX::XMVectorAdd(p1 , uVec);
	result = DirectX::XMVectorAdd(result , vVec);

	Info info;

	info.colliders = col;

	info.hitPoint = result;

	infos.push_back(info);
}

void RayCast::CheckHit()
{
	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	for (int i = 0; i < colList.size(); i++)
	{
		auto pair = colList[i]->GetTriangles();

		auto vertices = pair.second;
		auto index = pair.first;

		for (int j = 0; j < index.size(); j += 3)
		{
			auto p1 = vertices[index[j]];
			auto p2 = vertices[index[j + 1]];
			auto p3 = vertices[index[j + 2]];

			ParseRayInteract(p1, p2, p3, colList[i]);
		}
	}
}

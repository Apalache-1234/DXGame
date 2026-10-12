#include "PCH/pch.h"
#include "DebugRayCast.h"
#include "Utility.h"

DebugRayCast::DebugRayCast(std::shared_ptr<Entity> ent) : DebugRenderer(ent)
{
}

DebugRayCast::~DebugRayCast()
{
}

void DebugRayCast::Update()
{
	InitDebugRenderer();

	if (!InitIVB())
		std::cerr << "DebugRenderer‚Ì‰Šú‰»Ž¸”s" << std::endl;

	MapDVB();
}

void DebugRayCast::InitDebugRenderer()
{
	SetPrimitiveType(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

	CreateMesh(MeshLoader::SPHERE);

	instances.clear();

	for (int i = 0; i < rays.size(); i++)
	{
		Instance inst;

		auto dir = rays[i].GetDirection();
		auto yRot = DirectX::XMMatrixRotationY(-atan2(DirectX::XMVectorGetZ(dir), DirectX::XMVectorGetX(dir)));
		auto xAxis = DirectX::XMVector3Cross(dir , Utility::up);

		if (DirectX::XMVector3Equal(xAxis, DirectX::XMVectorZero()))
		{
			xAxis = Utility::forward;
		}

		xAxis = DirectX::XMVector3Normalize(xAxis);

		auto length = DirectX::XMVector2Length(DirectX::XMVectorSet(DirectX::XMVectorGetX(dir) , DirectX::XMVectorGetZ(dir) , 0 , 0));

		auto xQ = DirectX::XMQuaternionRotationAxis(xAxis, atan2(DirectX::XMVectorGetY(dir), DirectX::XMVectorGetX(length)));
		auto r = yRot * DirectX::XMMatrixRotationQuaternion(xQ);

		auto adjustPos = DirectX::XMVectorAdd(rays[i].GetStartPoint() , DirectX::XMVector3Transform( DirectX::XMVectorScale(Utility::right , rays[i].GetDistance() / 2 ), r) );

		auto t = DirectX::XMMatrixTranslationFromVector(adjustPos);

		auto s = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(rays[i].GetDistance() / 2, 0, 0, 0));

		inst.world = s*r*t;
		if (rays[i].GetIsHit())
			inst.color = XMFLOAT4(1, 0, 0, 1);
		else
			inst.color = XMFLOAT4(0, 1, 0, 1);

		instances.push_back(inst);

		if (rays[i].GetIsHit())
		{
			Instance instHit;

			auto points = rays[i].GetInfo();

			for (auto point : points)
			{
				instHit.world = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(0.05, 0.05, 0.05, 0)) * DirectX::XMMatrixTranslationFromVector(point.hitPoint);
				instHit.color = DirectX::XMFLOAT4(0, 0, 1, 1);

				instances.push_back(instHit);
			}
		}
	}

	instanceCount = instances.size();

	rays.clear();
}

void DebugRayCast::AddRayCast(RayCast ray)
{
	rays.push_back(ray);
}


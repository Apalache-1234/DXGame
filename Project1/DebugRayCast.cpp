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
	primitiveType = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;

	CreateMesh(MeshLoader::DEBUG_RAY);

	instances.clear();

	for (int i = 0; i < rays.size(); i++)
	{
		Instance inst;
		auto t =  DirectX::XMMatrixTranslationFromVector(rays[i].GetStartPoint());
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

		auto s = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(rays[i].GetDistance(), 0, 0, 0));

		inst.world = DirectX::XMMatrixTranspose(s*r*t);
		inst.color = XMFLOAT4(0, 1, 0, 1);

		instances.push_back(inst);

		if (rays[i].GetIsHit())
		{
			Instance instHit;

			instHit.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslationFromVector(rays[i].GetHitPoint()));
			instHit.color = XMFLOAT4(0.5, 0, 0, 1);

			instances.push_back(instHit);
		}
	}

	instanceCount = instances.size();

	rays.clear();
}

void DebugRayCast::AddRayCast(RayCast ray)
{
	rays.push_back(ray);
}


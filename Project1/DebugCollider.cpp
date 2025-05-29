#include "DebugCollider.h"

DebugColider::DebugColider(std::shared_ptr<Entity> ent) : DebugRenderer(ent)
{
}

DebugColider::DebugColider(std::shared_ptr<Entity> ent , Collider::ColliderType type) : DebugRenderer(ent)
{
	debugForType = type;

	std::cout << "adaafaw" << std::endl;
}

DebugColider::~DebugColider()
{
}

void DebugColider::Update()
{
	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	int sameCol = 0;

	for (int i = 0; i < colList.size(); i++)
	{
		if (colList[i]->GetColliderType() == debugForType)
		{
			sameCol++;
		}
	}

	if (sameCol != instanceCount)
	{
		InitDebugRenderer();

		if (!InitIVB())
			std::cerr << "DebugRenderer‚Ì‰Šú‰»Ž¸”s" << std::endl;
	}

	for (int i = 0; i < instanceCount; i++)
	{
		instances[i].world = DirectX::XMMatrixTranspose(cols[i]->GetTRSMatrix());
		if (cols[i]->isColliding)
			instances[i].color = XMFLOAT4(1, 0, 0, 1);
		else
			instances[i].color = XMFLOAT4(0, 1, 0, 1);

	}

	MapDVB();
}

void DebugColider::InitDebugRenderer()
{
	primitiveType = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;

	switch (debugForType)
	{
	case Collider::Cube:
		CreateMesh(MeshLoader::BOX);
		break;
	case Collider::Sphere:
		CreateMesh(MeshLoader::SPHERE);
		break;
	}

	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	instances.clear();

	for (int i = 0; i < colList.size(); i++)
	{
		if (colList[i]->GetColliderType() == debugForType)
		{
			Instance inst;
			inst.world = DirectX::XMMatrixTranspose(colList[i]->GetTRSMatrix());
			cols.emplace_back(colList[i]);
			if (colList[i]->isColliding)
				inst.color = XMFLOAT4(1, 0, 0, 1);
			else
				inst.color = XMFLOAT4(0, 1, 0, 1);

			instances.emplace_back(inst);
		}
	}

	instanceCount = instances.size();
}

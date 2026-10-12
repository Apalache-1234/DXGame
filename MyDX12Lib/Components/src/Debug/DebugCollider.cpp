#include "PCH/pch.h"
#include "DebugCollider.h"

DebugCollider::DebugCollider(std::shared_ptr<Entity> ent , Collider::ColliderType type) : DebugRenderer(ent)
{
	debugForType = type;
}

DebugCollider::~DebugCollider()
{
}

void DebugCollider::Update()
{
	auto colList = SystemManager::GetInstance().GetComponentList<Collider>();

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
		InitDebugColliderRenderer();

		if (!InitIVB())
			std::cerr << "DebugRenderer‚Ì‰Šú‰»Ž¸”s" << std::endl;
	}

	for (int i = 0; i < instanceCount; i++)
	{
		instances[i].world = cols[i]->GetTRSMatrix();
		if (cols[i]->isColliding)
			instances[i].color = XMFLOAT4(1, 0, 0, 1);
		else
			instances[i].color = XMFLOAT4(0, 1, 0, 1);

	}

	MapDVB();
}

void DebugCollider::InitDebugColliderRenderer()
{
	auto colList = SystemManager::GetInstance().GetComponentList<Collider>();

	instances.clear();
	cols.clear();

	for (int i = 0; i < colList.size(); i++)
	{
		if (colList[i]->GetColliderType() == debugForType)
		{
			Instance inst;
			inst.world = colList[i]->GetTRSMatrix();
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

void DebugCollider::InitDebugRenderer()
{
	SetPrimitiveType(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

	switch (debugForType)
	{
	case Collider::Cube:
		CreateMesh(MeshLoader::BOX);
		LoadPixelShader(L"DebugColBoxPixelShader.cso");
		break;
	case Collider::Sphere:
		CreateMesh(MeshLoader::SPHERE);
		break;
	}

	InitDebugColliderRenderer();
}

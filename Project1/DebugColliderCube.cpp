#include "DebugColliderCube.h"
#include "Collider.h"

DebugColiderCube::DebugColiderCube(std::shared_ptr<Entity> ent) : DebugRenderer(ent)
{
}

DebugColiderCube::~DebugColiderCube()
{
}

void DebugColiderCube::Update()
{
	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	int cube = 0;

	for (int i = 0; i < colList.size(); i++)
	{
		if (colList[i]->GetColliderType() == Collider::ColliderType::Cube)
		{
			cube++;
		}
	}

	if (cube != instanceCount)
	{
		InitDebugRenderer();

		if (!InitIVB())
			std::cerr << "DebugRenderer‚Ì‰Šú‰»Ž¸”s" << std::endl;
	}

	for (int i = 0; i < instanceCount; i++)
	{
		instances[i].world = DirectX::XMMatrixTranspose(colList[i]->GetTRSMatrix());
		if (colList[i]->isColliding)
			instances[i].color = XMFLOAT4(1, 0, 0, 1);
		else
			instances[i].color = XMFLOAT4(0, 1, 0, 1);

	}

	MapDVB();
}

void DebugColiderCube::InitDebugRenderer()
{
	primitiveType = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;

	auto colList = GameManager::GetInstance().GetComponentList<Collider>();

	instances.clear();

	for (int i = 0; i < colList.size(); i++)
	{
		if (colList[i]->GetColliderType() == Collider::ColliderType::Cube)
		{
			Instance inst;
			inst.world = DirectX::XMMatrixTranspose(colList[i]->GetTRSMatrix());
			if (colList[i]->isColliding)
				inst.color = XMFLOAT4(1, 0, 0, 1);
			else
				inst.color = XMFLOAT4(0, 1, 0, 1);

			instances.emplace_back(inst);
		}
	}

	instanceCount = instances.size();
}

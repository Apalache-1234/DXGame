#include "DebugOrigin.h"
#include "UIRenderer.h"

DebugOrigin::DebugOrigin(std::shared_ptr<Entity> ent) : DebugRenderer(ent)
{
}

DebugOrigin::~DebugOrigin()
{
}

void DebugOrigin::Update()
{
	auto ents = GameManager::GetInstance().GetEntityList();

	if (ents.size() != instanceCount)
	{
		InitDebugRenderer();

		if (!InitIVB())
			std::cerr << "DebugRenderer‚Ì‰Šú‰»Ž¸”s" << std::endl;
	}

	for (int i = 0; i < instanceCount; i++)
	{
		auto rotation_mat = DirectX::XMMatrixRotationQuaternion(ents[i]->GetRotationAsQuaternion());

		if (type == LOCAL)
			instances[i].world = rotation_mat * DirectX::XMMatrixTranslationFromVector(ents[i]->GetWorldPosition());
		else if (type == WORLD)
			instances[i].world = DirectX::XMMatrixTranslationFromVector(ents[i]->GetWorldPosition());
	}



	MapDVB();
}

void DebugOrigin::InitDebugRenderer()
{
	auto entList = GameManager::GetInstance().GetComponentList<UIRenderer>();

	CreateMesh(MeshLoader::BOX);

	instances.clear();

	for (int i = 0; i < entList.size(); i++)
	{
		Instance inst;

		inst.world = rotation_mat * DirectX::XMMatrixTranslationFromVector(entList[i]->GetWorldPosition());
		inst.color = DirectX::XMFLOAT4(0, 0, 0, 0);
		instances.emplace_back(inst);
	}

	instanceCount = instances.size();
}

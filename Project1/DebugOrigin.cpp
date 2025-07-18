#include "DebugOrigin.h"

DebugOrigin::DebugOrigin(std::shared_ptr<Entity> ent) : DebugRenderer(ent)
{
}

DebugOrigin::~DebugOrigin()
{
}

std::type_index DebugOrigin::GetBaseType()
{
	return typeid(DebugRenderer);
}

void DebugOrigin::Update()
{
	auto ents = SystemManager::GetInstance().GetEntityList();

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
	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace = {};
	dsDesc.BackFace = {};

	SetDepthStencilDesc(dsDesc);

	auto entList = SystemManager::GetInstance().GetEntityList();

	CreateMesh(MeshLoader::DEBUG_ORIGIN);

	instances.clear();

	for (int i = 0; i < entList.size(); i++)
	{
		Instance inst;

		auto rotation_mat = DirectX::XMMatrixRotationQuaternion(entList[i]->GetRotationAsQuaternion());

		inst.world = rotation_mat * DirectX::XMMatrixTranslationFromVector(entList[i]->GetWorldPosition());
		inst.color = DirectX::XMFLOAT4(0, 0, 0, 0);
		instances.emplace_back(inst);
	}

	instanceCount = instances.size();
}

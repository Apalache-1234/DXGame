#include "DebugDL.h"
#include "DirectionalLight.h"

DebugDirectionalLight::DebugDirectionalLight(std::shared_ptr<Entity> ent) : DebugRenderer(ent)
{
}

DebugDirectionalLight::~DebugDirectionalLight()
{
}

std::type_index DebugDirectionalLight::GetBaseType()
{
	return typeid(DebugRenderer);
}

void DebugDirectionalLight::Update()
{
	InitDebugRenderer();

	if (!InitIVB())
		std::cerr << "DebugDL‚Ì‰Šú‰»Ž¸”s" << std::endl;

	MapDVB();
}

void DebugDirectionalLight::InitDebugRenderer()
{
	SetPrimitiveType(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

	CreateMesh(MeshLoader::BOX);

	auto DLList = SystemManager::GetInstance().GetComponentList<DirectionalLight>();

	instances.clear();

	for (int i = 0; i < DLList.size(); i++)
	{
		Instance inst;

		auto dir = DLList[i]->GetLightDir();
		auto yRot = DirectX::XMMatrixRotationY(-atan2(DirectX::XMVectorGetZ(dir), DirectX::XMVectorGetX(dir)));
		auto xAxis = DirectX::XMVector3Cross(dir, Utility::up);

		if (DirectX::XMVector3Equal(xAxis, DirectX::XMVectorZero()))
		{
			xAxis = Utility::right;
		}

		xAxis = DirectX::XMVector3Normalize(xAxis);

		auto length = DirectX::XMVector2Length(DirectX::XMVectorSet(DirectX::XMVectorGetX(dir), DirectX::XMVectorGetZ(dir), 0, 0));

		auto xQ = DirectX::XMQuaternionRotationAxis(xAxis, atan2(DirectX::XMVectorGetY(dir), DirectX::XMVectorGetX(length)));
		auto r = yRot * DirectX::XMMatrixRotationQuaternion(xQ);

		auto adjustPos = DirectX::XMVectorAdd(DLList[i]->attachFrom.lock()->GetWorldPosition(), DirectX::XMVector3Transform(DirectX::XMVectorScale(Utility::right, 1), r));

		auto t = DirectX::XMMatrixTranslationFromVector(adjustPos);

		auto s = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(2, 0.1, 0.1, 0));

		inst.world = s * r * t;
		DirectX::XMStoreFloat4(&inst.color , DLList[i]->GetColor());

		instances.push_back(inst);
	}
	instanceCount = instances.size();
}

#include "DebugPointLight.h"
#include "PointLight.h"

DebugPointLight::DebugPointLight(std::shared_ptr<Entity> ent) : DebugRenderer::DebugRenderer(ent)
{
}

DebugPointLight::~DebugPointLight()
{
}

std::type_index DebugPointLight::GetBaseType()
{
	return typeid(DebugRenderer);
}

void DebugPointLight::Update()
{
	InitDebugPointLightRender();

	if (!InitIVB())
		std::cerr << "DebugDL‚Ì‰Šú‰»Ž¸”s" << std::endl;

	MapDVB();
}

void DebugPointLight::InitDebugPointLightRender()
{
	auto PLList = SystemManager::GetInstance().GetComponentList<PointLight>();

	instances.clear();

	for (int i = 0; i < PLList.size(); i++)
	{
		Instance inst;

		auto t = DirectX::XMMatrixTranslationFromVector(PLList[i]->attachFrom.lock()->GetWorldPosition());

		auto s = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorScale(DirectX::XMVectorSplatOne(), PLList[i]->GetRadius() * 2));

		inst.world = s * t;
		DirectX::XMStoreFloat4(&inst.color, PLList[i]->GetColor());

		instances.push_back(inst);
	}
	instanceCount = instances.size();
}

void DebugPointLight::InitDebugRenderer()
{
	LoadVertexShader(L"DebugPointLightVertexShader.cso");
	LoadPixelShader(L"DebugPointLightPixelShader.cso");

	CreateRenderPipeline();

	CreateMesh(MeshLoader::SPHERE);

	InitDebugPointLightRender();
}

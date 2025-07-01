#include "DebugSpotLight.h"
#include "SpotLight.h"
DebugSpotLight::DebugSpotLight(std::shared_ptr<Entity> ent) : DebugRenderer::DebugRenderer(ent)
{
}

DebugSpotLight::~DebugSpotLight()
{
}

void DebugSpotLight::Update()
{
	InitDebugRenderer();

	if (!InitIVB())
		std::cerr << "DebugSpotLight‚Ì‰Šú‰»Ž¸”s" << std::endl;

	MapDVB();
}

void DebugSpotLight::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->IASetVertexBuffers(1, 1, &IVBV);
}

void DebugSpotLight::InitDebugRenderer()
{
	CreateMesh(MeshLoader::SPHERE);

	auto DLList = GameManager::GetInstance().GetComponentList<SpotLight>();

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

		auto adjustPos = DirectX::XMVectorAdd(DLList[i]->attachFrom->GetWorldPosition(), DirectX::XMVector3Transform(DirectX::XMVectorScale(Utility::right, DLList[i]->GetLength()/2), r));

		auto t = DirectX::XMMatrixTranslationFromVector(adjustPos);

		auto s = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(DLList[i]->GetLength(), 0.1, 0.1, 0));

		inst.world = s * r * t;
		DirectX::XMStoreFloat4(&inst.color, DLList[i]->GetColor());

		instances.push_back(inst);

		inst.world = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorScale(DirectX::XMVectorSplatOne(), DLList[i]->GetLength() * 2)) * DirectX::XMMatrixTranslationFromVector(DLList[i]->attachFrom->GetWorldPosition());;

		instances.push_back(inst);
	}
	instanceCount = instances.size();

	LoadVertexShader(L"DebugPointLightVertexShader.cso");
	LoadPixelShader(L"DebugSpotLightPixelShader.cso");
	
	D3D12_INPUT_ELEMENT_DESC param = { "LIGHT_INDEX", 0, DXGI_FORMAT_R32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);
}
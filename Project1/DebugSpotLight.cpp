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
		std::cerr << "DebugSpotLightの初期化失敗" << std::endl;

	MapDVB();
}

void DebugSpotLight::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->IASetVertexBuffers(1, 1, &IVBV);
	cmdList->IASetVertexBuffers(2, 1, &LIBV);
}

bool DebugSpotLight::InitLightIndexBuffer()
{
	UINT bufferSize = sizeof(float) * lightIndex.size() + 1;

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = bufferSize;

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(LIB.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "インスタンスのライトインデックスバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DebugSpotLight::MapLIB()
{
	void* ptr = nullptr;

	hr = LIB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "インスタンスのスポットライトのインデックスが収納されたLIBマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, lightIndex.data(), sizeof(float) * lightIndex.size());

	LIB->Unmap(0, nullptr);

	return true;
}

bool DebugSpotLight::InitLIBV()
{
	LIBV.BufferLocation = LIB->GetGPUVirtualAddress();
	LIBV.SizeInBytes = sizeof(float) * lightIndex.size();
	LIBV.StrideInBytes = sizeof(float);

	return true;
}

void DebugSpotLight::InitDebugRenderer()
{
	CreateMesh(MeshLoader::SPHERE);

	auto DLList = GameManager::GetInstance().GetComponentList<SpotLight>();

	instances.clear();
	lightIndex.clear();

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
		lightIndex.emplace_back(-1);

		inst.world = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorScale(DirectX::XMVectorSplatOne(), DLList[i]->GetLength() * 2)) * DirectX::XMMatrixTranslationFromVector(DLList[i]->attachFrom->GetWorldPosition());;

		instances.push_back(inst);
		lightIndex.emplace_back(i);
	}
	instanceCount = instances.size();

	LoadVertexShader(L"DebugSpotLightVertexShader.cso");
	LoadPixelShader(L"DebugSpotLightPixelShader.cso");
	
	D3D12_INPUT_ELEMENT_DESC param = { "LIGHT_INDEX", 0, DXGI_FORMAT_R32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	AddInputElementDesc(param);

	InitLightIndexBuffer();
	MapLIB();
	InitLIBV();

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = false;

	SetDepthStencilDesc(dsDesc);
}
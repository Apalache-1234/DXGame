#include "CardScript.h"
#include "HandScript.h"

CardScript::CardScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

CardScript::~CardScript()
{
}

void CardScript::Init(DeckScript::Card card)
{
	if (card.pat != DeckScript::EMPTY)
	{
		attachFrom->SetWorldScale(DirectX::XMVectorSet(1, 1, 1, 0));

		sRend->ApplyTextureOnMesh(1, Renderer::DIFFUSE, cardTexSc->GetCardTexture(card));
	}
	else
		attachFrom->SetWorldScale(DirectX::XMVectorSet(0,0,0,0));
}

void CardScript::DrawBack()
{
	Utility::Debug(attachFrom->GetLocalPosition());
	Utility::Debug(handSc->GetCardLocalPos(attachFrom->GetAsChildIndex()));

	*twDrawBack = tweenSystem->DoTween(attachFrom->GetLocalPosition(), handSc->GetCardLocalPos(attachFrom->GetAsChildIndex()), EasingType::EaseOutSine, 0.2f);;
}

void CardScript::SetDrawBackTween(shared_ptr<Tween<XMVECTOR>>* tw)
{
	twDrawBack = tw;
}

void CardScript::OnStart()
{
	auto ent = systemManager->GetEntityByName("CardTex");

	col = attachFrom->AddComponent<Collider>();
	col->scale = DirectX::XMVectorSet(1.26,0.0203, 1.79 , 1);

	cardTexSc = ent->GetComponent<CardTextureScript>();

	sRend = attachFrom->AddComponent<SkinnedRenderer>();
	sRend->CreateMesh(MeshLoader::CARD);
	auto mat = sRend->GetMaterial(1);
	mat.UVSCALE = XMFLOAT2(-1, 1);
	sRend->SetMaterial(1, mat);
	sRend->ApplyTextureOnMesh(0, Renderer::DIFFUSE, textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));

	ent = std::dynamic_pointer_cast<Entity>(attachFrom->GetParent());

	if (ent != nullptr)
		handSc = ent->GetComponent<HandScript>();

	sRend->LoadPixelShader(L"CardPixelShader.cso");

	InitCB();

	LinkCBVToDescHeap();

	MapCB();

	std::function<void(ComPtr<ID3D12GraphicsCommandList>)> f = [=](ComPtr<ID3D12GraphicsCommandList> i) { i->SetGraphicsRootConstantBufferView(13, CBView.BufferLocation);};

	sRend->AddCommandListFunction(f);

}

void CardScript::MapCB()
{
	info.isHit = isOnRay;

	HRESULT hr;

	void* ptr = nullptr;

	hr = CB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "CBのマッピング失敗" << std::endl;
	}

	memcpy(ptr, &info, sizeof(CardInfo));

	CB->Unmap(0, nullptr);
}

void CardScript::InitCB()
{
	HRESULT hr;

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

	desc.Width = sizeof(CardInfo);

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
		IID_PPV_ARGS(CB.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "SquareWipeのCBのリソース作成失敗" << std::endl;
	}
}

bool CardScript::LinkCBVToDescHeap()
{
	CBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetHandle());

	CBView.BufferLocation = CB->GetGPUVirtualAddress();

	CBView.SizeInBytes = sizeof(CardInfo);

	DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&CBView, CBHandle->cpuHandle);

	return true;
}


DeckScript::SIDE CardScript::GetSide()
{
	if (!handSc)
		return DeckScript::TABLE;

	return handSc->GetSide();
}

void CardScript::OnUpdate()
{
	if (gameM == nullptr)
	{
		auto ent = systemManager->GetEntityByName("GameManager");

		gameM = ent->GetComponent<GameManagerScript>();
	}

	MapCB();

	isOnRay = false;
}

void CardScript::OnRayCast()
{
}
#include "TableDeckScript.h"
#include "CardScript.h"
#include "CardScript.h"
#include "Collider.h"
#include "Utility.h"

TableDeckScript::TableDeckScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TableDeckScript::~TableDeckScript()
{
}

void TableDeckScript::OnStart()
{
	for (int i = 0;i < pool;i++)
	{
		cardPool.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<CardScript>("CardPool"));

		auto rend = cardPool.back()->GetComponent<SkinnedRenderer>();
		auto mat = rend->GetMaterial(1);
		mat.UVSCALE = XMFLOAT2(1, -1);
		rend->SetMaterial(1, mat);

		cardPool.back()->SetWorldScale(DirectX::XMVectorZero());
	}

	tws.resize(pool);

	auto col = attachFrom.lock()->AddComponent<Collider>();
	col->scale = DirectX::XMVectorSet(1.26, 0.0203, 1.79, 1);

	sRend = attachFrom.lock()->AddComponent<SkinnedRenderer>();
	sRend->CreateMesh(MeshLoader::CARD);
	sRend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));

	auto ent = systemManager->GetEntityByName("CardTex");

	cardTex = ent->GetComponent<CardTextureScript>();

	attachFrom.lock()->SetWorldScale(DirectX::XMVectorZero());

	auto mat = sRend->GetMaterial(1);
	mat.UVSCALE = XMFLOAT2(1, -1);
	sRend->SetMaterial(1, mat);

	sRend->LoadPixelShader(L"CardPixelShader.cso");

	InitCB();

	LinkCBVToDescHeap();

	MapCB();

	std::function<void(ComPtr<ID3D12GraphicsCommandList>)> f = [=](ComPtr<ID3D12GraphicsCommandList> i) { i->SetGraphicsRootConstantBufferView(13, CBView.BufferLocation);};

	sRend->AddCommandListFunction(f);
}

void TableDeckScript::PlaceCard(DeckScript::Card card , DirectX::XMVECTOR fromPos)
{
	currentTopCard = card;

	curPool++;
	curPool = curPool % 3;

	auto sc = cardPool[curPool]->GetComponent<CardScript>();

	sc->Init(card);

	cardNum++;

	auto worldPos = attachFrom.lock()->GetWorldPosition();
	auto adPos = DirectX::XMVectorSet(0,0.0203f * cardNum,0,0);
	auto toPos = DirectX::XMVectorAdd(worldPos , adPos);

	cardPool[curPool]->SetWorldScale(DirectX::XMVectorSplatOne());

	tws[curPool] = tweenSystem->DoTween(fromPos, toPos, EasingType::EaseOutSine, 0.25f);


	tws[curPool]->OnComplete([=, targetIndex = curPool, cardN = cardNum]() {
		attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(1, 1 + cardN, 1, 1));
		cardPool[curPool]->SetWorldScale(DirectX::XMVectorZero());
		sRend->ApplyTextureOnMesh(1, Renderer::TEXTURE_TYPE::DIFFUSE, cardTex->GetCardTexture(card));
		});

	tws[curPool]->OnUpdate([=, targetIndex = curPool]() {cardPool[targetIndex]->SetWorldPosition(tws[curPool]->GetValue());});
}

void TableDeckScript::MapCB()
{
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

void TableDeckScript::InitCB()
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
		std::cerr << "TableDeckScriptのCBのリソース作成失敗" << std::endl;
	}
}

bool TableDeckScript::LinkCBVToDescHeap()
{
	CBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetHandle());

	CBView.BufferLocation = CB->GetGPUVirtualAddress();

	CBView.SizeInBytes = sizeof(CardInfo);

	DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&CBView, CBHandle->cpuHandle);

	return true;
}

bool TableDeckScript::IsPlaceable(DeckScript::Card card)
{
	if (currentTopCard.num + 1 == card.num || currentTopCard.num - 1 == card.num)
	{
		return true;
	}

	if (currentTopCard.num == 13)
	{
		if (card.num == 1)
		{
			return true;
		}
	}

	if (currentTopCard.num == 1)
	{
		if (card.num == 13)
		{
			return true;
		}
	}
}

void TableDeckScript::OnRayCast()
{
	if (gameM->GetIsGrabbedCard() && IsPlaceable(gameM->GetCurrentCard()))
	{
		info.isHit = true;
	}
}

void TableDeckScript::OnUpdate()
{
	if (gameM == nullptr)
	{
		auto ent = systemManager->GetEntityByName("GameManager");

		gameM = ent->GetComponent<GameManagerScript>();
	}

	MapCB();

	info.isHit = false;
}

bool TableDeckScript::GetIsOnRayCast()
{
	return info.isHit;
}

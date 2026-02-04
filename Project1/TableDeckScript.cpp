#include "TableDeckScript.h"
#include "CardScript.h"

TableDeckScript::TableDeckScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TableDeckScript::~TableDeckScript()
{
}

void TableDeckScript::OnStart()
{
	for (int i = 0;i < 3;i++)
	{
		cardPool.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<CardScript>("CardPool"));

		cardPool[i]->SetWorldScale(DirectX::XMVectorZero());
	}

	sRend = attachFrom->AddComponent<SkinnedRenderer>();
	sRend->CreateMesh(MeshLoader::CARD);
	sRend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));

	auto ent = systemManager->GetEntityByName("CardTex");

	cardTex = ent->GetComponent<CardTextureScript>();

	attachFrom->SetWorldScale(DirectX::XMVectorZero());

	auto mat = sRend->GetMaterial(1);
	mat.UVSCALE = XMFLOAT2(-1, 1);
	sRend->SetMaterial(1, mat);
}

void TableDeckScript::PlaceCard(DeckScript::Card card , DirectX::XMVECTOR fromPos)
{
	curPool++;
	curPool = curPool % 3;

	cardPool[curPool]->SetWorldScale(DirectX::XMVectorSplatOne());

	auto sc = cardPool[curPool]->GetComponent<CardScript>();

	sc->Init(card);

	cardNum++;

	auto worldPos = attachFrom->GetWorldPosition();
	auto adPos = DirectX::XMVectorSet(0,0.0203f * cardNum,0,0);
	auto toPos = DirectX::XMVectorAdd(worldPos , adPos);

	tws.emplace_back(tweenSystem->DoTween(fromPos , toPos , EasingType::EaseOutSine , 0.4f));
	tws.back()->OnComplete([=, targetIndex = curPool]() {
		cardPool[targetIndex]->SetWorldScale(DirectX::XMVectorZero());
		attachFrom->SetWorldScale(DirectX::XMVectorSet(1, 1 + cardNum, 1, 1));
		sRend->ApplyTextureOnMesh(1, Renderer::TEXTURE_TYPE::DIFFUSE, cardTex->GetCardTexture(card));
		});
	tws.back()->OnUpdate([=, targetIndex = curPool , currentTween = tws.back()]() {cardPool[targetIndex]->SetWorldPosition(currentTween->GetValue());});
}

void TableDeckScript::OnUpdate()
{
}

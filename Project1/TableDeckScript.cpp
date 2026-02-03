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
	sRend = attachFrom->AddComponent<SkinnedRenderer>();
	sRend->CreateMesh(MeshLoader::CARD);
	sRend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));

	auto ent = systemManager->GetEntityByName("CardTex");

	cardTex = ent->GetComponent<CardTextureScript>();

	attachFrom->SetWorldScale(DirectX::XMVectorZero());

	sRend->CreateMesh(MeshLoader::CARD);
	auto mat = sRend->GetMaterial(1);
	mat.UVSCALE = XMFLOAT2(-1, 1);
	sRend->SetMaterial(1, mat);
}

void TableDeckScript::PlaceCard(DeckScript::Card card , DirectX::XMVECTOR fromPos)
{
	auto ent = sceneManager->GetCurrentGameScene()->CreateEntity<CardScript>("Card");

	auto sc = ent->GetComponent<CardScript>();

	sc->Init(card);

	cardNum++;

	auto worldPos = attachFrom->GetWorldPosition();
	auto adPos = DirectX::XMVectorSet(0,0.0203f * cardNum,0,0);
	auto toPos = DirectX::XMVectorAdd(worldPos , adPos);

	tws.emplace_back(tweenSystem->DoTween(fromPos , toPos , EasingType::EaseOutSine , 0.4f));
	tws.back()->OnComplete([=]() {
		ent->Destroy(); 
		attachFrom->SetWorldScale(DirectX::XMVectorSet(1, 1 + cardNum, 1, 1));
		sRend->ApplyTextureOnMesh(1, Renderer::TEXTURE_TYPE::DIFFUSE, cardTex->GetCardTexture(card));
		});
	tws.back()->OnUpdate([=]() {ent->SetWorldPosition(tws.back()->GetValue());});
}

void TableDeckScript::OnUpdate()
{
}

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
}

DeckScript::SIDE CardScript::GetSide()
{
	return handSc->GetSide();
}

void CardScript::OnUpdate()
{
	if (gameM == nullptr)
	{
		auto ent = systemManager->GetEntityByName("GameManager");

		gameM = ent->GetComponent<GameManagerScript>();
	}
}

bool CardScript::GetIsOnRay()
{
	return isOnRay;
}

void CardScript::OnRayCast()
{
	if (KeyInput::GetInstance().GetKeyDown(MOUSE_LEFT))
		isOnRay = true;
}
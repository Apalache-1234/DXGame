#include "HandScript.h"
#include "DeckScript.h"

HandScript::HandScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

HandScript::~HandScript()
{
}

DirectX::XMVECTOR GetCardLocalPos(int idx)
{
	return DirectX::XMVectorSet(idx * 1.9f - 2.9f, 0.7f, 2.1, 0);
}

void HandScript::Init(DeckScript::SIDE side)
{
	this->side = side;

	std::string name;

	std::shared_ptr<Entity> deckEnt;

	if(this->side == DeckScript::PLAYER)
		deckEnt = systemManager->GetEntityByName("PlayerDeck");
	else
	{
		deckEnt = systemManager->GetEntityByName("CPUDeck");
	}

	deckSc = deckEnt->GetComponent<DeckScript>();

	if (this->side == DeckScript::PLAYER)
		name += "Player";
	else
		name += "Cpu";

	name += "HandCard";

	for (int i = 0;i < handNum;i++)
	{
		name += std::to_string(i + 1);
		cardEnts.emplace_back(sceneManager->GetCurrentGameScene()->CreateEntity<SkinnedRenderer>(name));
	}

	for (int i = 0;i < cardEnts.size();i++)
	{
		attachFrom->AddChild(cardEnts[i]);

		cardRends.emplace_back(cardEnts[i]->GetComponent<SkinnedRenderer>());
		cardRends.back()->CreateMesh(MeshLoader::CARD);
		auto mat = cardRends.back()->GetMaterial(1);
		mat.UVSCALE = XMFLOAT2(-1,1);
		cardRends.back()->SetMaterial(1 , mat);
		cardRends.back()->ApplyTextureOnMesh(0, Renderer::DIFFUSE, textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));

		cardEnts[i]->SetLocalPosition(GetCardLocalPos(i));
		cardEnts[i]->SetWorldRotation(DirectX::XMVectorSet(0, 180, 0, 0));
	}

	attachFrom->SetWorldPosition(DirectX::XMVectorSet(0,0,0,0));
	if (this->side == DeckScript::CPU)
	{
		attachFrom->SetWorldRotation(DirectX::XMVectorSet(0, 180, 0, 0));
	}
}

void HandScript::OnStart()
{
	std::string diamondStr = "_of_diamonds";
	std::string clubStr = "_of_clubs";
	std::string heartStr = "_of_hearts";
	std::string spadeStr = "_of_spades";

	for (int i = 0;i < 4;i++)
	{
		for (int j = 1;j < 14;j++)
		{
			ComPtr<ID3D12Resource> resource;

			std::string filePath = "Textures/Cards/";

			if (1 == j)
				filePath += "ace";
			else if (11 == j)
				filePath += "jack";
			else if (12 == j)
				filePath += "queen";
			else if (13 == j)
				filePath += "king";
			else
				filePath += to_string(j);

			if (0 == i)
				filePath += heartStr;
			else if (1 == i)
				filePath += diamondStr;
			else if (2 == i)
				filePath += spadeStr;
			else
				filePath += clubStr;

			filePath += ".dds";

			DeckScript::Card card = DeckScript::Card((DeckScript::PATTERN)i , j);

			resource = textureLoader->LoadTexture(filePath);

			cardTexs[card] = resource;
		}
	}

}

void HandScript::PlaceCard(DeckScript::Card card)
{
	for (int i = 0;i < handNum;i++)
	{
		if (cards[i].pat == DeckScript::EMPTY)
		{
			cards[i] = card;

			cardRends[i]->ApplyTextureOnMesh(1,Renderer::DIFFUSE , cardTexs[card]);

			cardTws[i] = tweenSystem->DoTween(cardEnts[i]->GetWorldToLocalPos(deckSc->GetTopPos()), GetCardLocalPos(i), EasingType::EaseOutSine , 0.3f);

			return;
		}
	}


}

bool HandScript::IsAnyEmpty()
{
	bool empty = false;

	for (int i = 0;i < handNum;i++)
	{
		if (cards[i].pat == DeckScript::EMPTY)
		{
			empty = true;
		}
	}

	return empty;
}

void HandScript::OnUpdate()
{
	if (KeyInput::GetInstance().GetKey(D))
	{
		if (IsAnyEmpty())
		{
			PlaceCard(deckSc->DrawCard());
		}
	}

	for (int i = 0;i < handNum;i++)
	{
		if (cards[i].pat == DeckScript::EMPTY)
			cardEnts[i]->SetWorldScale(DirectX::XMVectorSet(0, 0, 0, 0));
		else
			cardEnts[i]->SetWorldScale(DirectX::XMVectorSet(1, 1, 1, 0));
		if (cardTws[i] != nullptr)
		{
			cardEnts[i]->SetLocalPosition(cardTws[i]->GetValue());
		}
	}
}

#include "HandScript.h"
#include "DeckScript.h"
#include "CardScript.h"


HandScript::HandScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

HandScript::~HandScript()
{
}

DirectX::XMVECTOR HandScript::GetCardLocalPos(int idx)
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

		cardEnts[i]->SetLocalPosition(GetCardLocalPos(i));
		cardEnts[i]->SetWorldRotation(DirectX::XMVectorSet(0, 180, 0, 0));
		cardScs.emplace_back(cardEnts[i]->AddComponent<CardScript>());

		cardScs.back()->SetDrawBackTween(&cardTws[i]);
	}

	attachFrom->SetWorldPosition(DirectX::XMVectorSet(0,0,0,0));
	if (this->side == DeckScript::CPU)
	{
		attachFrom->SetWorldRotation(DirectX::XMVectorSet(0, 180, 0, 0));
	}
}

void HandScript::OnStart()
{
}


void HandScript::PlaceCard()
{
	if (!IsAnyEmpty())
	{
		return;
	}

	auto card = deckSc->DrawCard();

	for (int i = 0;i < handNum;i++)
	{
		if (cards[i].pat == DeckScript::EMPTY)
		{
			cards[i] = card;

			cardScs[i]->Init(card);


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

DeckScript::SIDE HandScript::GetSide()
{
	return side;
}

shared_ptr<DeckScript> HandScript::GetUsingDeck()
{
	return deckSc;
}

void HandScript::OnUpdate()
{
	if (KeyInput::GetInstance().GetKey(D))
	{
		PlaceCard();
	}

	for (int i = 0;i < handNum;i++)
	{
		if (cards[i].pat == DeckScript::EMPTY)
			cardEnts[i]->SetWorldScale(DirectX::XMVectorSet(0, 0, 0, 0));
		else
			cardEnts[i]->SetWorldScale(DirectX::XMVectorSet(1, 1, 1, 0));
		if (cardTws[i] != nullptr && !cardScs[i]->isGrabbed)
		{
			cardEnts[i]->SetLocalPosition(cardTws[i]->GetValue());
		}
	}
}

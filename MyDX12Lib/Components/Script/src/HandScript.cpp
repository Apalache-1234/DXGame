#include "PCH/pch.h"
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
		attachFrom.lock()->AddChild(cardEnts[i]);

		cardEnts[i]->SetLocalPosition(GetCardLocalPos(i));
		cardEnts[i]->SetWorldRotation(DirectX::XMVectorSet(0, 180, 0, 0));
		cardScs.emplace_back(cardEnts[i]->AddComponent<CardScript>());

		cardScs.back()->SetDrawBackTween(&cardTws[i]);
		cardScs.back()->Init(DeckScript::Card());
	}

	attachFrom.lock()->SetWorldPosition(DirectX::XMVectorSet(0,0,0,0));
	if (this->side == DeckScript::CPU)
	{
		attachFrom.lock()->SetWorldRotation(DirectX::XMVectorSet(0, 180, 0, 0));
	}

	auto ent = systemManager->GetEntityByName("TableDeck1");
	tableDeck1Sc = ent->GetComponent<TableDeckScript>();
	ent = systemManager->GetEntityByName("TableDeck2");
	tableDeck2Sc = ent->GetComponent<TableDeckScript>();
}

void HandScript::OnStart()
{
}

std::pair < DeckScript::Card, DirectX::XMVECTOR> HandScript::DrawPlaceableCard()
{
	for (int i = 0;i < handNum;i++)
	{
		if (tableDeck1Sc->IsPlaceable(cardScs[i]->GetCurrentCard()) || tableDeck2Sc->IsPlaceable(cardScs[i]->GetCurrentCard()))
		{
			auto card = cardScs[i]->GetCurrentCard();
			auto pos = cardScs[i]->attachFrom.lock()->GetWorldPosition();

			cardScs[i]->Init(DeckScript::Card());

			return std::make_pair(card, pos);
		}
	}
	return std::make_pair(DeckScript::Card(), DirectX::XMVectorZero());
}

bool HandScript::HasPlaceableCard()
{
	for (int i = 0;i < handNum;i++)
	{
		if (tableDeck1Sc->IsPlaceable(cardScs[i]->GetCurrentCard()) || tableDeck2Sc->IsPlaceable(cardScs[i]->GetCurrentCard()))
			return true;
	}
	return false;
}

void HandScript::PlaceCard()
{
	if (!IsAnyEmpty())
	{
		return;
	}

	auto card = deckSc->DrawCard();

	if (card.pat == DeckScript::EMPTY)
		return;

	for (int i = 0;i < handNum;i++)
	{
		if (cardScs[i]->GetCurrentCard().pat == DeckScript::EMPTY)
		{
			cardScs[i]->Init(card);

			cardEnts[i]->SetLocalPosition(cardEnts[i]->GetWorldToLocalPos(deckSc->GetTopPos()));

			cardTws[i] = tweenSystem->DoTween(cardEnts[i]->GetWorldToLocalPos(deckSc->GetTopPos()), GetCardLocalPos(i), EasingType::EaseOutSine , 0.3f);

			return;
		}
	}


}

bool HandScript::IsAllEmpty()
{
	bool empty = true;

	for (int i = 0;i < handNum;i++)
	{
		if (cardScs[i]->GetCurrentCard().pat != DeckScript::EMPTY)
		{
			empty = false;
		}
	}

	return empty;
}

bool HandScript::IsAnyEmpty()
{
	bool empty = false;

	for (int i = 0;i < handNum;i++)
	{
		if (cardScs[i]->GetCurrentCard().pat == DeckScript::EMPTY)
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

std::pair < DeckScript::Card, DirectX::XMVECTOR> HandScript::DrawRandomCard()
{
	auto list = cardScs;

	auto result = std::remove_if(list.begin(), list.end(),
		[](shared_ptr<CardScript>& e) { return e->GetCurrentCard().pat == DeckScript::EMPTY ; });

	list.erase(result,list.end());

	if (list.empty())
		return std::make_pair(DeckScript::Card() , DirectX::XMVectorZero());

	int ind = Utility::GetRandomInt(0 , list.size() - 1);

	auto card = list[ind]->GetCurrentCard();
	auto pos = list[ind]->attachFrom.lock()->GetWorldPosition();

	list[ind]->Init(DeckScript::Card());

	return std::make_pair(card , pos);
}

void HandScript::OnUpdate()
{
	if (gameM == nullptr)
	{
		auto ent = systemManager->GetEntityByName("GameManager");

		gameM = ent->GetComponent<GameManagerScript>();
	}

	for (int i = 0;i < handNum;i++)
	{
		if (cardTws[i] != nullptr && !cardScs[i]->isGrabbed)
		{
			cardEnts[i]->SetLocalPosition(cardTws[i]->GetValue());
		}
	}


	if (IsAnyEmpty() && gameM->GetIsSetupEnd() && !deckSc->IsEmpty())
	{
		PlaceCard();
	}
}

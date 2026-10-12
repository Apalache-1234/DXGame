#include "PCH/pch.h"
#include "CPUScript.h"
#include "TableDeckScript.h"
#include "GameManagerScript.h"
#include "PlaceCardUIScript.h"

CPUScript::CPUScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

CPUScript::~CPUScript()
{
}

void CPUScript::OnStart()
{
	cHandSc = systemManager->GetEntityByName("CPUHand")->GetComponent<HandScript>();
	cDeckSc = systemManager->GetEntityByName("CPUDeck")->GetComponent<DeckScript>();

	auto ent = systemManager->GetEntityByName("TableDeck1");
	tableDeck1Sc = ent->GetComponent<TableDeckScript>();
	ent = systemManager->GetEntityByName("TableDeck2");
	tableDeck2Sc = ent->GetComponent<TableDeckScript>();

	ent = systemManager->GetEntityByName("PlaceCardUI");
	placeCardUISc = ent->GetComponent<PlaceCardUIScript>();
}

void CPUScript::OnUpdate()
{
	if (gameM == nullptr)
	{
		auto ent = systemManager->GetEntityByName("GameManager");

		gameM = ent->GetComponent<GameManagerScript>();
	}

	if (!gameM->GetIsSetupEnd() || placeCardUISc->GetAnimStatus() != PlaceCardUIScript::END || cHandSc->IsAllEmpty())
	{
		collapsedTime = 0;
		return;
	}

	collapsedTime += systemManager->GetGameDeltaTime();

	auto val = (float)Utility::GetRandomInt(0,100) / (float)100;

	val = std::pow(val , 2.0f);

	float thinkInterval = std::lerp(thinkInterval1 , thinkInterval2 , val);

	if (collapsedTime > thinkInterval)
	{
		if (cHandSc->HasPlaceableCard())
		{
			auto card = cHandSc->DrawPlaceableCard();

			if (tableDeck1Sc->IsPlaceable(card.first))
				tableDeck1Sc->PlaceCard(card.first , card.second);
			else if (tableDeck2Sc->IsPlaceable(card.first))
				tableDeck2Sc->PlaceCard(card.first, card.second);
		}

		collapsedTime = 0;
	}
}

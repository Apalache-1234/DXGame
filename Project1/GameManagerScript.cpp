#include "GameManagerScript.h"
#include "CardScript.h"
#include "Entity.h"
#include "RayCast.h"
#include "PlaceCardUIScript.h"
#include "GameStartingScript.h"

GameManagerScript::GameManagerScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

GameManagerScript::~GameManagerScript()
{
}

void GameManagerScript::OnStart()
{
	auto ent = systemManager->GetEntityByName("SquareWipe");

	wipeSc = ent->GetComponent<SquareWipeScript>();

	ent = systemManager->GetEntityByName("PlayerHand");
	pHandSc = ent->GetComponent<HandScript>();
	ent = systemManager->GetEntityByName("CPUHand");
	cHandSc = ent->GetComponent<HandScript>();

	pDeckSc = pHandSc->GetUsingDeck();
	cDeckSc = cHandSc->GetUsingDeck();

	ent = systemManager->GetEntityByName("TableDeck1");
	tableDeck1Sc = ent->GetComponent<TableDeckScript>();
	ent = systemManager->GetEntityByName("TableDeck2");
	tableDeck2Sc = ent->GetComponent<TableDeckScript>();

	ent = systemManager->GetEntityByName("PlaceCardUI");
	placeCardUISc = ent->GetComponent<PlaceCardUIScript>();

	ent = systemManager->GetEntityByName("GameStarting");
	gameStartingSc = ent->GetComponent<GameStartingScript>();
	
}

DirectX::XMVECTOR GameManagerScript::GetCurrectCardPos()
{
	if (!selectCard)
		return DirectX::XMVectorZero();

	return selectCard->GetWorldPosition();
}

void GameManagerScript::PlaceCardsOnTableDecks()
{
	auto pCard = pDeckSc->DrawCard();

	if (pCard.pat != DeckScript::EMPTY)
	{
		tableDeck1Sc->PlaceCard(pCard, pDeckSc->GetTopPos());
	}
	else
	{
		auto pHandCard = pHandSc->DrawRandomCard();

		if(pHandCard.first.pat != DeckScript::EMPTY)
			tableDeck1Sc->PlaceCard(pHandCard.first, pHandCard.second);

	}

	auto cCard = cDeckSc->DrawCard();

	if (cCard.pat != DeckScript::EMPTY)
	{
		tableDeck2Sc->PlaceCard(cCard, cDeckSc->GetTopPos());
	}
	else
	{
		auto cHandCard = cHandSc->DrawRandomCard();

		if (cHandCard.first.pat != DeckScript::EMPTY)
			tableDeck2Sc->PlaceCard(cHandCard.first, cHandCard.second);

	}
}

bool GameManagerScript::GetIsSetupEnd()
{
	return isSetupEnd;
}

GameManagerScript::RESULT GameManagerScript::CheckIsAllEmptyEachSide()
{
	if (pDeckSc->IsEmpty() && pHandSc->IsAllEmpty())
		return PLAYER_W;
	else if (cDeckSc->IsEmpty() && cHandSc->IsAllEmpty())
		return CPU_W;
	else if (pDeckSc->IsEmpty() && pHandSc->IsAllEmpty() && cDeckSc->IsEmpty() && cHandSc->IsAllEmpty())
		return DRAW;

	return NOT_YET;
}

void GameManagerScript::OnUpdate()
{
	if (wipeSc->GetIsWipeEnd())
	{
		startDrawTime += systemManager->GetGameDeltaTime();

		while (startDrawTime > drawInterval && drawNum != 0)
		{
			startDrawTime -= drawInterval;

			drawNum--;

			if (drawNum > 1)
			{
				pHandSc->PlaceCard();
				cHandSc->PlaceCard();
			}
			else if (drawNum == 1)
			{
				PlaceCardsOnTableDecks();

				gameStartingSc->StartAnim([=]() {isSetupEnd = true;});
			}
		}
	}

	if (isGameFinished)
		return;

	if (!isSetupEnd || placeCardUISc->GetAnimStatus() != PlaceCardUIScript::END || CheckIsAllEmptyEachSide() != NOT_YET)
	{
		if (selectCard)
		{
			selectCard->GetComponent<CardScript>()->DrawBack();

			selectCard->GetComponent<CardScript>()->isGrabbed = false;

			selectCard = nullptr;
		}

		if (CheckIsAllEmptyEachSide() != NOT_YET)
			isGameFinished = true;

		return;
	}



	if (!pHandSc->HasPlaceableCard() && !cHandSc->HasPlaceableCard() && placeCardUISc->GetAnimStatus() == PlaceCardUIScript::END)
	{
		placeCardUISc->StartAnim(
			[=]() { PlaceCardsOnTableDecks(); }
		);

		return;
	}

	auto mousePos = KeyInput::GetInstance().GetMousePos();

	auto mousePosXm = DirectX::XMLoadFloat2(&mousePos);

	auto dir = DirectX::XMVectorSubtract(Camera::GetInstance().GetPosScreenToWorld(mousePosXm), Camera::GetInstance().GetPosition());

	auto infos = RayCast::RayCast(Camera::GetInstance().GetPosition(), DirectX::XMVector3Normalize(dir), 100).GetInfo();

	if (infos.size() != 0)
	{
		auto cardSc = infos[0].colliders->attachFrom.lock()->GetComponent<CardScript>();

		if (cardSc && cardSc->GetSide() == DeckScript::PLAYER)
		{
			cardSc->isOnRay = true;

			if (KeyInput::GetInstance().GetKey(Key::MOUSE_LEFT) && cardSc->GetCurrentCard().pat != DeckScript::EMPTY)
			{
				selectCard = infos.front().colliders->attachFrom.lock();
				cardSc->isGrabbed = true;
				if (cardSc->GetSide() != DeckScript::PLAYER)
					selectCard = nullptr;
			}
		}
	}

	mousePosXm = DirectX::XMVectorSetZ(mousePosXm, 4.8);

	if (selectCard)
	{
		mousePosXm = DirectX::XMVectorSetZ(mousePosXm, 4.8);

		selectCard->SetWorldPosition(Camera::GetInstance().GetPosScreenToWorld(mousePosXm));
	}

	if (selectCard && KeyInput::GetInstance().GetKeyUp(Key::MOUSE_LEFT))
	{
		selectCard->GetComponent<CardScript>()->DrawBack();

		selectCard->GetComponent<CardScript>()->isGrabbed = false;

		bool isPlaced = false;

		if (tableDeck1Sc->GetIsOnRayCast())
		{
			tableDeck1Sc->PlaceCard(selectCard->GetComponent<CardScript>()->GetCurrentCard() , Camera::GetInstance().GetPosScreenToWorld(mousePosXm));

			isPlaced = true;
		}
		else if (tableDeck2Sc->GetIsOnRayCast())
		{
			tableDeck2Sc->PlaceCard(selectCard->GetComponent<CardScript>()->GetCurrentCard(), Camera::GetInstance().GetPosScreenToWorld(mousePosXm));

			isPlaced = true;
		}

		if (isPlaced)
			selectCard->GetComponent<CardScript>()->Init(DeckScript::Card());

		selectCard = nullptr;
	}
}

DeckScript::Card GameManagerScript::DrawCurrentCard()
{
	auto card = selectCard->GetComponent<CardScript>()->GetCurrentCard();

	selectCard->GetComponent<CardScript>()->Init(DeckScript::Card());

	selectCard = nullptr;

	return card;
}

DeckScript::Card GameManagerScript::GetCurrentCard()
{
	return selectCard->GetComponent<CardScript>()->GetCurrentCard();
}

bool GameManagerScript::GetIsGrabbedCard()
{
	return selectCard != nullptr;
}

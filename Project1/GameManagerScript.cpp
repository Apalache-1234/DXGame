#include "GameManagerScript.h"
#include "CardScript.h"
#include "Entity.h"
#include "RayCast.h"

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
	
}

void GameManagerScript::PlaceCardsOnTableDecks()
{
	auto pCard = pDeckSc->DrawCard();
	auto cCard = cDeckSc->DrawCard();

	if (pCard.pat == DeckScript::EMPTY || cCard.pat == DeckScript::EMPTY)
	{
		
	}
	else
	{
		tableDeck1Sc->PlaceCard(pCard, pDeckSc->GetTopPos());
		tableDeck2Sc->PlaceCard(cCard, cDeckSc->GetTopPos());
	}
}

void GameManagerScript::OnUpdate()
{
	auto mousePos = KeyInput::GetInstance().GetMousePos();

	auto mousePosXm = DirectX::XMLoadFloat2(&mousePos);

	auto dir = DirectX::XMVectorSubtract(Camera::GetInstance().GetPosScreenToWorld(mousePosXm), Camera::GetInstance().GetPosition());

	auto info = RayCast::RayCast(Camera::GetInstance().GetPosition(), DirectX::XMVector3Normalize(dir), 100).GetInfo();

	if (KeyInput::GetInstance().GetKey(D))
	{
		PlaceCardsOnTableDecks();
	}

	if (info.size() != 0)
	{
		if (KeyInput::GetInstance().GetKey(Key::MOUSE_LEFT))
		{
			selectCard = info.front().colliders->attachFrom;

			if (selectCard->GetComponent<CardScript>()->GetSide() != DeckScript::PLAYER)
				selectCard = nullptr;
		}
	}

	if (selectCard)
	{
		mousePosXm = DirectX::XMVectorSetZ(mousePosXm, 5);
		selectCard->SetWorldPosition(Camera::GetInstance().GetPosScreenToWorld(mousePosXm));
	}
		

	if (selectCard && KeyInput::GetInstance().GetKeyUp(Key::MOUSE_LEFT))
	{
		mousePosXm = DirectX::XMVectorSetZ(mousePosXm, 5);

		selectCard->SetWorldPosition(Camera::GetInstance().GetPosScreenToWorld(mousePosXm));

		selectCard->GetComponent<CardScript>()->DrawBack();

		selectCard = nullptr;
	}

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
			}
		}


	}
}

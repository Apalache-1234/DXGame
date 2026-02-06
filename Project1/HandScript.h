#ifndef HAND_SCRIPT_H
#define HAND_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "DeckScript.h"
#include "CardTextureScript.h"
#include "SquareWipeScript.h"

class Entity;
class CardScript;
class GameManagerScript;
class TableDeckScript;

class HandScript : public ScriptComponent
{
public:
	HandScript(std::shared_ptr<Entity> ent);

	~HandScript();

	DirectX::XMVECTOR GetCardLocalPos(int idx);

	void Init(DeckScript::SIDE side);

	void OnStart() override;

	std::pair < DeckScript::Card, DirectX::XMVECTOR> DrawPlaceableCard();

    bool HasPlaceableCard();

	void PlaceCard();

	bool IsAllEmpty();

	bool IsAnyEmpty();

	DeckScript::SIDE GetSide();

	shared_ptr<DeckScript> GetUsingDeck();

	std::pair < DeckScript::Card, DirectX::XMVECTOR> DrawRandomCard();

	void OnUpdate() override;
protected:
private:
	DeckScript::SIDE side;

	shared_ptr<DeckScript> deckSc;

	std::vector<shared_ptr<CardScript>> cardScs;

	shared_ptr<SquareWipeScript> wipeSc;
	shared_ptr<GameManagerScript> gameM;

	std::shared_ptr<TableDeckScript>  tableDeck1Sc;
	std::shared_ptr<TableDeckScript>  tableDeck2Sc;

	const static int handNum = 5;

	std::shared_ptr<SkinnedRenderer> sRend;

	std::vector<std::shared_ptr<Entity>> cardEnts;
	std::vector<std::shared_ptr<SkinnedRenderer>> cardRends;
	std::shared_ptr<Tween < XMVECTOR>> cardTws[handNum];

	Material mat;

	shared_ptr<CardTextureScript> cardTex;
};

#endif
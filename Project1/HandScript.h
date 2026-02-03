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

class HandScript : public ScriptComponent
{
public:
	HandScript(std::shared_ptr<Entity> ent);

	~HandScript();

	DirectX::XMVECTOR GetCardLocalPos(int idx);

	void Init(DeckScript::SIDE side);

	void OnStart() override;

	void PlaceCard();

	bool IsAnyEmpty();

	DeckScript::SIDE GetSide();

	shared_ptr<DeckScript> GetUsingDeck();

	void OnUpdate() override;
protected:
private:
	DeckScript::SIDE side;

	shared_ptr<DeckScript> deckSc;

	std::vector<shared_ptr<CardScript>> cardScs;

	shared_ptr<SquareWipeScript> wipeSc;

	const static int handNum = 5;

	DeckScript::Card cards[handNum];

	std::shared_ptr<SkinnedRenderer> sRend;

	std::vector<std::shared_ptr<Entity>> cardEnts;
	std::vector<std::shared_ptr<SkinnedRenderer>> cardRends;
	std::shared_ptr<Tween < XMVECTOR>> cardTws[handNum];

	Material mat;

	shared_ptr<CardTextureScript> cardTex;
};

#endif
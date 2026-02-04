#ifndef TABLE_DECK_SCRIPT_H
#define TABLE_DECK_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "DeckScript.h"
#include "Tween.h"
#include "CardTextureScript.h"

class Entity;

class TableDeckScript : public ScriptComponent
{
public:
	TableDeckScript(std::shared_ptr<Entity> ent);

	~TableDeckScript();

	void OnStart() override;

	void PlaceCard(DeckScript::Card card, DirectX::XMVECTOR fromPos);

	void OnUpdate() override;
protected:
private:
	shared_ptr<CardTextureScript> cardTex;

	std::shared_ptr<SkinnedRenderer> sRend;

	std::vector< DeckScript::Card> cards;

	std::vector < std::shared_ptr<Tween < XMVECTOR>>> tws;

	std::vector<shared_ptr<Entity>> cardPool;

	UINT cardNum = 0;

	UINT curPool = 0;

	Material mat;
};

#endif
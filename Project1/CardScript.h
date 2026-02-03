#ifndef CARD_SCRIPT_H
#define CARD_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "DeckScript.h"
#include "CardTextureScript.h"
#include "GameManagerScript.h"
#include "Tween.h"
#include "PERadicalBlur.h"
#include "Collider.h"

class Entity;
class HandScript;

class CardScript : public ScriptComponent
{
public:
	CardScript(std::shared_ptr<Entity> ent);

	~CardScript();

	void Init(DeckScript::Card card);

	void DrawBack();

	void SetDrawBackTween(shared_ptr<Tween<XMVECTOR>>* tw);

	void OnStart() override;

	DeckScript::SIDE GetSide();

	void OnUpdate() override;

	bool GetIsOnRay();

	void OnRayCast() override;
private:
	DeckScript::Card ver;

	std::function<void()> fTween;

	shared_ptr<Tween<XMVECTOR>>* twDrawBack = nullptr;

	shared_ptr<CardTextureScript> cardTexSc;
	shared_ptr<HandScript> handSc;
	shared_ptr<Collider> col;

	shared_ptr<GameManagerScript> gameM;

	bool isOnRay = false;

	std::shared_ptr<SkinnedRenderer> sRend;
};

#endif
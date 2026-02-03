#ifndef HAND_SCRIPT_H
#define HAND_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "DeckScript.h"

class Entity;

class HandScript : public ScriptComponent
{
public:
	HandScript(std::shared_ptr<Entity> ent);

	~HandScript();

	void Init(DeckScript::SIDE side);

	void OnStart() override;

	void PlaceCard(DeckScript::Card card);

	bool IsAnyEmpty();

	void OnUpdate() override;
protected:
private:
	struct CardHash {
		std::size_t operator()(const DeckScript::Card& k) const {
			return std::hash<int>()((int)k.pat) ^ (std::hash<int>()(k.num) << 1);
		}
	};

	DeckScript::SIDE side;

	shared_ptr<DeckScript> deckSc;

	std::unordered_map<DeckScript::Card, ComPtr<ID3D12Resource> , CardHash> cardTexs;

	const static int handNum = 5;

	DeckScript::Card cards[handNum];

	std::shared_ptr<SkinnedRenderer> sRend;

	std::vector<std::shared_ptr<Entity>> cardEnts;
	std::vector<std::shared_ptr<SkinnedRenderer>> cardRends;
	std::shared_ptr<Tween < XMVECTOR>> cardTws[handNum];

	Material mat;
};

#endif
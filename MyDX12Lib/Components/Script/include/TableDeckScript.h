#ifndef TABLE_DECK_SCRIPT_H
#define TABLE_DECK_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "DeckScript.h"
#include "Tween.h"
#include "CardTextureScript.h"

class Entity;
class GameManagerScript;

class TableDeckScript : public ScriptComponent
{
public:
	TableDeckScript(std::shared_ptr<Entity> ent);

	~TableDeckScript();

	void OnStart() override;

	void PlaceCard(DeckScript::Card card, DirectX::XMVECTOR fromPos);

	void OnUpdate() override;

    bool GetIsOnRayCast();

	void OnRayCast() override;

	bool IsPlaceable(DeckScript::Card card);
protected:
private:
	struct alignas(256) CardInfo
	{
		float isHit = false;

		XMFLOAT3 padding1;
	};

	shared_ptr<CardTextureScript> cardTex;

	std::shared_ptr<SkinnedRenderer> sRend;

	std::shared_ptr<GameManagerScript> gameM;

	std::vector< DeckScript::Card> cards;

	std::vector < std::shared_ptr<Tween < XMVECTOR>>> tws;

	std::vector<shared_ptr<Entity>> cardPool;

	CardInfo info;

	UINT cardNum = 0;

	UINT curPool = 0;
	const UINT pool = 3;

	DeckScript::Card currentTopCard;

	void MapCB();

	void InitCB();

	bool LinkCBVToDescHeap();

	ComPtr<ID3D12Resource> CB;
	std::unique_ptr<PtrHandles> CBHandle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC CBView;

	Material mat;
};

#endif
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

	void MapCB();

	void InitCB();

	bool LinkCBVToDescHeap();

	DeckScript::SIDE GetSide();

	void OnUpdate() override;

	void OnRayCast() override;

	bool isOnRay = false;
	bool isGrabbed = false;
private:
	struct alignas(256) CardInfo
	{
		float isHit = false;

		XMFLOAT3 padding1;
	};

	CardInfo info;

	DeckScript::Card ver;

	std::function<void()> fTween;

	shared_ptr<Tween<XMVECTOR>>* twDrawBack = nullptr;

	shared_ptr<CardTextureScript> cardTexSc;
	shared_ptr<HandScript> handSc;
	shared_ptr<Collider> col;

	shared_ptr<GameManagerScript> gameM;

	ComPtr<ID3D12Resource> CB;
	std::unique_ptr<DescriptorHeapManager::PtrHandles> CBHandle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC CBView;

	std::shared_ptr<SkinnedRenderer> sRend;
};

#endif
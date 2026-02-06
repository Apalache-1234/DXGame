#ifndef PLACE_CARD_H
#define PLACE_CARD_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;
class UIRenderer;

class PlaceCardUIScript : public ScriptComponent
{
public:
	enum AnimStatus
	{
		FIRST, SECOND, THIRD , FOURTH, END
	};

	PlaceCardUIScript(std::shared_ptr<Entity> ent);

	~PlaceCardUIScript();

	void OnStart() override;

	void MapCB();

	void InitCB();

	bool LinkCBVToDescHeap();

	void StartAnim(std::function<void()> whenEndF);

	AnimStatus GetAnimStatus();

	void OnUpdate() override;
protected:
private:

	struct alignas(256) UIInfo
	{
		float time = 0;

		XMFLOAT3 padding1;

		DirectX::XMVECTOR color;
	};

	UIInfo info;

	float time = 0;

	float interval = 0.7;

	AnimStatus animPat = END;

	std::shared_ptr<UIRenderer> sRend;
	std::shared_ptr<Entity> readyEnt;
	std::shared_ptr<UIRenderer> readyRend;

	Material mat;

	bool isFuncExec = false;

	std::function<void()> f;

	ComPtr<ID3D12Resource> CB;
	std::unique_ptr<DescriptorHeapManager::PtrHandles> CBHandle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC CBView;
};

#endif
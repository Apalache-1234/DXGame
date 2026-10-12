#ifndef SQUARE_WIPE_SCRIPT_H
#define SQUARE_WIPE_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PERadicalBlur.h"
#include "UIRenderer.h"

class Entity;

class SquareWipeScript : public ScriptComponent
{
public:
	SquareWipeScript(std::shared_ptr<Entity> ent);

	~SquareWipeScript();

	void OnStart() override;

	void OnUpdate() override;

	bool GetIsWipeEnd();
protected:
private:
	struct alignas(256) SquareInfo
	{
		XMMATRIX rotate = DirectX::XMMatrixIdentity();
		float scale = 0;
		XMFLOAT3 pad2;
	};

	SquareInfo info;

	std::vector<std::shared_ptr<Tween < float>>> tws;

	std::shared_ptr<UIRenderer> rend;

	bool isWipeEnd = false;

	ComPtr<ID3D12Resource> CB;
	std::unique_ptr<PtrHandles> CBHandle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC CBView;

	void MapCB();
	void InitCB();
	bool LinkCBVToDescHeap();
};

#endif
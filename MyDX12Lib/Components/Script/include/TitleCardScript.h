#ifndef TITLE_CARD_SCRIPT_H
#define TITLE_CARD_SCRIPT_H


#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PENoiseFilter.h"

class Entity;

class TitleCardScript : public ScriptComponent
{
public:
	TitleCardScript(std::shared_ptr<Entity> ent);

	~TitleCardScript();

	void OnStart() override;

	void OnUpdate() override;
private:
	std::shared_ptr<SkinnedRenderer> rend;

	float time;

	std::vector<ComPtr<ID3D12Resource>> texs;
};

#endif
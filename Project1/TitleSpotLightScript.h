#ifndef TITLE_SPOT_LIGHT_SCRIPT_H
#define TITLE_SPOT_LIGHT_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;

class TitleSpotLightScript : public ScriptComponent
{
public:
	TitleSpotLightScript(std::shared_ptr<Entity> ent);

	~TitleSpotLightScript();

	void OnStart() override;

	void LightUpward();

	void OnUpdate() override;
private:
	std::shared_ptr<SpotLight> slComp;

	std::shared_ptr<Tween < XMVECTOR>> tw;
	std::shared_ptr<Tween < XMVECTOR>> tw2;
	std::shared_ptr<Tween < XMVECTOR>> tw3;

	std::shared_ptr<Tween < XMVECTOR>> tw4;
};

#endif
#ifndef TITLE_NOISE_SCRIPT_H
#define TITLE_NOISE_SCRIPT_H


#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PENoiseFilter.h"

class Entity;

class TitleNoiseScript : public ScriptComponent
{
public:
	TitleNoiseScript(std::shared_ptr<Entity> ent);

	~TitleNoiseScript();

	void OnStart() override;

	void OnUpdate() override;
private:
	std::shared_ptr<PENoiseFilter> noisePE;

	std::shared_ptr<Tween <float>> tw;
};

#endif
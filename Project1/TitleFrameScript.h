#ifndef TITLE_FRAME_SCRIPT_H
#define TITLE_FRAME_SCRIPT_H


#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PENoiseFilter.h"

class Entity;

class TitleFrameScript : public ScriptComponent
{
public:
	TitleFrameScript(std::shared_ptr<Entity> ent);

	~TitleFrameScript();

	void OnStart() override;

	void OnUpdate() override;
private:
	std::shared_ptr<SkinnedRenderer> rend;
};

#endif
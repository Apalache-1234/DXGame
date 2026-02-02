#ifndef TITLE_FRAME_SCRIPT_H
#define TITLE_FRAME_SCRIPT_H


#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "PENoiseFilter.h"
#include "Collider.h"

class Entity;

class TitleFrameScript : public ScriptComponent
{
public:
	TitleFrameScript(std::shared_ptr<Entity> ent);

	~TitleFrameScript();

	void OnStart() override;

	void OnUpdate() override;

	void OnRayCast() override;

	enum FLAG
	{
		START, EXIT
	};

	FLAG flag = START;
private:
	std::shared_ptr<SkinnedRenderer> rend;

	Material mat;

	bool isOnRay = false;

	bool isPressed = false;

	std::shared_ptr<Collider> col;

	float time;
};

#endif
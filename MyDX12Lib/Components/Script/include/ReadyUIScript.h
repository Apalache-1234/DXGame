#ifndef READY_UI_SCRIPT_H
#define READY_UI_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;
class UIRenderer;

class ReadyUIScript : public ScriptComponent
{
public:
	ReadyUIScript(std::shared_ptr<Entity> ent);

	~ReadyUIScript();

	void OnStart() override;

	void OnUpdate() override;
protected:
private:
	std::shared_ptr<UIRenderer> sRend;

	Material mat;
};

#endif
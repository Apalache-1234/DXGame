#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include <vector>
#include <list>
#include <typeindex>
#include <memory>
#include <iostream>
#include <unordered_map>
#include "Component.h"
#include "TweenSystem.h"
#include "MeshLoader.h"
#include "TextureLoader.h"
#include "SceneManager.h"
#include "DirectionalLight.h"
#include "Entity.h"
#include "Camera.h"
#include "KeyInput.h"
#include "SkinnedRenderer.h"

class Entity;

class ScriptComponent : public Component
{
public:
	ScriptComponent(std::shared_ptr<Entity> ent);

	virtual ~ScriptComponent();

	virtual void OnMousePointerIn();

	virtual void OnMousePointerOn();

	virtual void OnMousePointerClick();

	virtual void OnMousePointerOut();

	virtual void OnStart();

	virtual void OnUpdate();

	virtual void OnFixedUpdate();

	virtual void OnDestroy();

	void Init() override;

protected:
	TweenSystem* tweenSystem;

	SystemManager* systemManager;

	MeshLoader* meshLoader;

	TextureLoader* textureLoader;

	SceneManager* sceneManager;

	Camera* camera;
};

#endif
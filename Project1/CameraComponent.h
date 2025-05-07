#ifndef CAMERACOMPONENT_H
#define CAMERACOMPONENT_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "Entity.h"
#include "Camera.h"


class CameraComponent : public Camera , public Component
{
public:
	CameraComponent(std::shared_ptr<Entity> ent);

	~CameraComponent();

	void SetCameraTransform(DirectX::XMVECTOR position , DirectX::XMVECTOR rotation);

protected:

};

#endif
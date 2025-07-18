#ifndef CAMERACOMPONENT_H
#define CAMERACOMPONENT_H

#include <memory>
#include <DirectXMath.h>
#include "Component.h"
#include "Camera.h"



class CameraComponent : public Camera , public Component
{
public:
	CameraComponent(std::shared_ptr<Entity> ent);

	~CameraComponent();

	void SetCameraTransform(DirectX::XMVECTOR position , DirectX::XMVECTOR rotation);

	std::type_index GetBaseType() override;
protected:

};

#endif
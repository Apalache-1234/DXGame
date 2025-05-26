#include "Camera.h"
#include "Utility.h"
#include <iostream>
#include "Entity.h"
#include <vector>
#include <list>

const float Camera::nearZ = 1.0f;
const float Camera::farZ = 1000.0f;
const float Camera::aspect = (float)16 / (float)9;
const float Camera::fov = DirectX::XMConvertToRadians(70);

Camera::Camera()
{
}

Camera::~Camera()
{
}

std::unique_ptr<Camera> Camera::instance = nullptr;

Camera& Camera::GetInstance()
{
    if (!instance) {
        instance.reset(new Camera());
    }
    return *instance;
}

DirectX::XMVECTOR Camera::GetPosition()
{
    return camPosition;
}

DirectX::XMVECTOR Camera::GetRotation()
{
    return  camRotation;
}

DirectX::XMVECTOR Camera::GetTargetVec()
{
    return targetVec;
}

DirectX::XMVECTOR Camera::GetTargetPos()
{
    return targetPos;
}

void Camera::SetCameraPosition(DirectX::XMVECTOR position)
{
    camPosition = position;

    targetPos = DirectX::XMVectorAdd(camPosition, targetVec);
}

void Camera::SetCameraRotation(DirectX::XMVECTOR rotation)
{
    camRotation = rotation;

    DirectX::XMVECTOR camQuaternion = DirectX::XMQuaternionRotationRollPitchYaw(
        DirectX::XMConvertToRadians(DirectX::XMVectorGetX(camRotation)),
        DirectX::XMConvertToRadians(DirectX::XMVectorGetY(camRotation)),
        DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(camRotation))
    );

    targetVec = DirectX::XMVector3Rotate(forward, camQuaternion);

    targetPos = DirectX::XMVectorAdd(camPosition, targetVec);
}

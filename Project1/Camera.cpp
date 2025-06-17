#include "Camera.h"
#include "Utility.h"
#include <iostream>
#include "Entity.h"
#include <vector>
#include "Window.h"
#include "KeyInput.h"
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

DirectX::XMVECTOR Camera::GetPosWorldToScreen(DirectX::XMVECTOR pos)
{
    auto view = XMMatrixLookAtLH(Camera::GetInstance().GetPosition(), Camera::GetInstance().GetTargetPos(), DirectX::XMVectorSet(0, 1, 0, 0));
    auto proj = XMMatrixPerspectiveFovLH(Camera::fov, Camera::GetInstance().aspect, Camera::nearZ, Camera::farZ);

    auto screenPos = DirectX::XMVector3TransformCoord(pos , view);
    screenPos = DirectX::XMVector4Transform(screenPos , proj);

    screenPos = DirectX::XMVectorScale(screenPos, 1 / DirectX::XMVectorGetW(screenPos));

    screenPos = DirectX::XMVectorSetX(screenPos , DirectX::XMVectorGetX(screenPos) * Window::width / 2);
    screenPos = DirectX::XMVectorSetY(screenPos , DirectX::XMVectorGetY(screenPos) * Window::height / 2);

    return DirectX::XMVectorSet(DirectX::XMVectorGetX(screenPos), DirectX::XMVectorGetY(screenPos) ,0 , 0);
}

DirectX::XMVECTOR Camera::GetPosScreenToWorld(DirectX::XMVECTOR pos)
{
    auto x = DirectX::XMVectorGetX(pos) / Window::width * 2 - 1;
    auto y = DirectX::XMVectorGetY(pos) / Window::height * 2 - 1;

    auto z = std::clamp(DirectX::XMVectorGetZ(pos) , nearZ , farZ);

    auto worldPos = DirectX::XMVectorSet(x * nearZ, y * nearZ, 0, nearZ);

    auto viewInverse = XMMatrixLookAtLH(GetPosition(), GetTargetPos(), DirectX::XMVectorSet(0, 1, 0, 0));
    auto projInverse = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    viewInverse = DirectX::XMMatrixInverse(nullptr, viewInverse);
    projInverse = DirectX::XMMatrixInverse(nullptr, projInverse);

    worldPos = DirectX::XMVector4Transform(worldPos, projInverse);
    worldPos = DirectX::XMVector4Transform(worldPos, viewInverse);

    DirectX::XMVECTOR cameraPos = GetPosition();
    DirectX::XMVECTOR rayDir = XMVector3Normalize(XMVectorSubtract(worldPos, cameraPos));

    return DirectX::XMVectorAdd(cameraPos, DirectX::XMVectorScale(rayDir , z));
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

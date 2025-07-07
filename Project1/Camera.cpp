#include "Camera.h"
#include "Utility.h"
#include <iostream>
#include "Entity.h"
#include <vector>
#include "KeyInput.h"
#include <list>

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

float Camera::GetFov()
{
    return fov;
}

float Camera::GetNearZ()
{
    return nearZ;
}

float Camera::GetFarZ()
{
    return farZ;
}

float Camera::GetAspect()
{
    return aspect;
}

void Camera::SetFov(float v)
{
    v = fmin(fmax(v, 0.01f), Utility::PI);

    fov = v;
}

void Camera::SetNearZ(float v)
{
    v = max(v, 0.001);

    nearZ = v;
}

void Camera::SetFarZ(float v)
{
    v = max(v, nearZ + 0.1);

    farZ = v;
}

void Camera::SetAspect(float v)
{
    aspect = fabs(v);
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

#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <DirectXMath.h>
#include <assimp/matrix4x4.h>
#include <random>
#include <Windows.h>
#include "DescriptorHeapManager.h"
#undef TRANSPARENT

#define PtrHandle std::unique_ptr<DescriptorHeapManager::PtrHandles>

using namespace Microsoft::WRL;

namespace Utility
{

    static const DirectX::XMVECTOR zero3D = DirectX::XMVectorSet(0, 0, 0, 0);

    static const DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
    static const DirectX::XMVECTOR down = DirectX::XMVectorSet(0, -1, 0, 0);
    static const DirectX::XMVECTOR right = DirectX::XMVectorSet(1, 0, 0, 0);
    static const DirectX::XMVECTOR left = DirectX::XMVectorSet(-1, 0, 0, 0);
    static const DirectX::XMVECTOR forward = DirectX::XMVectorSet(0, 0, 1, 0);
    static const DirectX::XMVECTOR backward = DirectX::XMVectorSet(0, 0, -1, 0);

    static const float ep = 1e-3f;

    static const float PI = 3.14;

    DirectX::XMMATRIX ConvertAiMatrix4x4ToXMMATRIX(aiMatrix4x4 mat);

    int GetRandomInt(int min, int max);

    void Debug(const std::string text);

    void Debug(const DirectX::XMVECTOR xm);

    void Debug(const DirectX::XMFLOAT3 xm);

    void Debug(const DirectX::XMMATRIX xm);

    UINT Align256(UINT size);

    DirectX::XMMATRIX CreateTransMat(const DirectX::XMVECTOR xm1, const DirectX::XMVECTOR xm2, const DirectX::XMVECTOR xm3);

    DirectX::XMVECTOR GetZYXMultiplyQuaternion(DirectX::XMVECTOR x, DirectX::XMVECTOR y , DirectX::XMVECTOR z);}

struct alignas(256) Transform
{
    //ワールド変換行列
    DirectX::XMMATRIX world;

    //ビュー行列
    DirectX::XMMATRIX view;

    //投影行列
    DirectX::XMMATRIX projection;

    //メッシュに適用する色
    DirectX::XMFLOAT4 color;
};

struct alignas(256) LightInfo
{
    DirectX::XMFLOAT3 ambientLightColor;

    float padding1;

    XMFLOAT4 DirectionalLightNum;

    XMFLOAT4 PointLightNum;

    XMFLOAT4 SpotLightNum;

    XMVECTOR cameraPos;

    float cascadeShadowCount;

    XMFLOAT3 padding2;

    XMFLOAT3 cascadeShadowDist;

    float padding3;
};

struct alignas(256) TimeInfo
{
    float GameTime = 0;

    DirectX::XMFLOAT3 padding1;

    float GameDeltaTime = 0;

    DirectX::XMFLOAT3 padding2;

    float RealTime = 0;

    DirectX::XMFLOAT3 padding3;

    float RealDeltaTime = 0;

    DirectX::XMFLOAT3 padding4;
};

enum class SamplerType
{
    DEFAULT,
    SHADOW_MAP
};

struct Material
{
    DirectX::XMFLOAT3 DIFFUSE = DirectX::XMFLOAT3(1,1,1);
    DirectX::XMFLOAT3 SPECULAR = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT3 AMBIENT = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT3 REFLECTIVE = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT3 TRANSPARENT = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT3 EMISSIVE = DirectX::XMFLOAT3(1, 1, 1);
    DirectX::XMFLOAT2 UVPOS = XMFLOAT2(0,0);
    DirectX::XMFLOAT2 UVSCALE = XMFLOAT2(1, 1);
    DirectX::XMFLOAT4 TEXELSIZE = XMFLOAT4(123, 123, 123, 123);
};

#endif
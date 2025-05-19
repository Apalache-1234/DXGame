#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <DirectXMath.h>
#include <assimp/matrix4x4.h>
#include <random>

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

    DirectX::XMMATRIX ConvertAiMatrix4x4ToXMMATRIX(aiMatrix4x4 mat);

    int GetRandomInt(int min, int max);

    void Debug(const std::string text);

    void Debug(const DirectX::XMVECTOR xm);

    void Debug(const DirectX::XMMATRIX xm);

    DirectX::XMMATRIX CreateTransMat(const DirectX::XMVECTOR xm1, const DirectX::XMVECTOR xm2, const DirectX::XMVECTOR xm3);
}

#endif
#include "Utility.h"

namespace Utility
{
    DirectX::XMMATRIX ConvertAiMatrix4x4ToXMMATRIX(aiMatrix4x4 mat)
    {
        DirectX::XMMATRIX matrix
        {
            mat.a1,mat.b1,mat.c1,mat.d1,
            mat.a2,mat.b2,mat.c2,mat.d2,
            mat.a3,mat.b3,mat.c3,mat.d3,
            mat.a4,mat.b4,mat.c4,mat.d4
        };

        return matrix;
    }

    int GetRandomInt(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(min, max);

        return dist(gen);
    }

    void Debug(const std::string text)
    {
        std::cout << text << std::endl;
    }

    void Debug(const DirectX::FXMVECTOR xm)
    {
        float x = DirectX::XMVectorGetX(xm);
        float y = DirectX::XMVectorGetY(xm);
        float z = DirectX::XMVectorGetZ(xm);
        float w = DirectX::XMVectorGetW(xm);

        std::cout << "X : " << x << " || Y : " << y << " || Z : " << z << " || W : " << w << std::endl;
    }

    void Debug(const DirectX::XMFLOAT3 xm)
    {
        float x = xm.x;
        float y = xm.y;
        float z = xm.z;


        std::cout << "X : " << x << " || Y : " << y << " || Z : " << z << std::endl;
    }


    void Debug(const DirectX::FXMMATRIX xm)
    {
        for (int i = 0;i < 4;i++)
        {
            std::cout << "[ " << xm.r[i].m128_f32[0] << " | " << xm.r[i].m128_f32[1] << " | " << xm.r[i].m128_f32[2] << " | " << xm.r[i].m128_f32[3] <<  " ]" << std::endl;
        }

        std::cout << std::endl;
    }


    DirectX::XMMATRIX CreateTransMat(const DirectX::FXMVECTOR xm1 , const DirectX::GXMVECTOR xm2 , const DirectX::HXMVECTOR xm3)
    {
        DirectX::XMVECTOR* ptrs = new DirectX::XMVECTOR[3];

        DirectX::XMMATRIX mat = DirectX::XMMATRIX();

        ptrs[0] = xm1;
        ptrs[1] = xm2;
        ptrs[2] = DirectX::XMVectorScale(xm3 , -1);

        for (int i = 0;i < 3;i++)
        {
            float x = DirectX::XMVectorGetX(ptrs[i]);
            float y = DirectX::XMVectorGetY(ptrs[i]);
            float z = DirectX::XMVectorGetZ(ptrs[i]);

            mat.r[0].m128_f32[i] = x;
            mat.r[1].m128_f32[i] = y;
            mat.r[2].m128_f32[i] = z;
        }

        mat.r[3].m128_f32[3] = 1;

        delete[] ptrs;

        return mat;
    }

    DirectX::XMVECTOR GetZYXMultiplyQuaternion(DirectX::FXMVECTOR x, DirectX::GXMVECTOR y, DirectX::HXMVECTOR z)
    {
        DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(x, y); 
        qResult = DirectX::XMQuaternionMultiply(qResult, z);

        return qResult;
    }
}
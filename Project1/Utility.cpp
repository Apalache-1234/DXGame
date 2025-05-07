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

    void Debug(const std::string text)
    {
        std::cout << text << std::endl;
    }

    void Debug(const DirectX::XMVECTOR xm)
    {
        float x = DirectX::XMVectorGetX(xm);
        float y = DirectX::XMVectorGetY(xm);
        float z = DirectX::XMVectorGetZ(xm);
        float w = DirectX::XMVectorGetW(xm);

        std::cout << "X : " << x << " || Y : " << y << " || Z : " << z << " || W : " << w << std::endl;
    }


    void Debug(const DirectX::XMMATRIX xm)
    {
        for (int i = 0;i < 4;i++)
        {
            std::cout << "[ " << xm.r[i].m128_f32[0] << " | " << xm.r[i].m128_f32[1] << " | " << xm.r[i].m128_f32[2] << " | " << xm.r[i].m128_f32[3] <<  " ]" << std::endl;
        }

        std::cout << std::endl;
    }


    DirectX::XMMATRIX CreateTransMat(const DirectX::XMVECTOR xm1 , const DirectX::XMVECTOR xm2 , const DirectX::XMVECTOR xm3)
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
}
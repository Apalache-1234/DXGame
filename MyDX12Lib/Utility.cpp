#include "PCH/pch.h"
#include "Utility.h"
#include "SystemManager.h"
#include "json.hpp"

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

    nlohmann::json SaveVectorToJson(nlohmann::json& json, const std::string_view name, DirectX::XMVECTOR vec,  SaveVecNum vecNum)
    {
        json[name]["X"] = DirectX::XMVectorGetX(vec);
        json[name]["Y"] = DirectX::XMVectorGetY(vec);

        if(vecNum >= SaveVecNum::Triple)
            json[name]["Z"] = DirectX::XMVectorGetZ(vec);
        if (vecNum >= SaveVecNum::Quad)
            json[name]["W"] = DirectX::XMVectorGetW(vec);
  

        return json;
    }

    nlohmann::json SaveVectorToJson(nlohmann::json& json, const std::string_view name, DirectX::XMFLOAT2 vec)
    {
        json[name]["X"] = vec.x;
        json[name]["Y"] = vec.y;

        return json;
    }

    nlohmann::json SaveVectorToJson(nlohmann::json& json, const std::string_view name, DirectX::XMFLOAT3 vec)
    {
        json[name]["X"] = vec.x;
        json[name]["Y"] = vec.y;
        json[name]["Z"] = vec.z;

        return json;
    }

    nlohmann::json SaveVectorToJson(nlohmann::json& json, const std::string_view name, DirectX::XMFLOAT4 vec)
    {
        json[name]["X"] = vec.x;
        json[name]["Y"] = vec.y;
        json[name]["Z"] = vec.z;
        json[name]["W"] = vec.w;

        return json;
    }

    DirectX::XMVECTOR LoadJsonToVec(const nlohmann::json& json, const std::string_view name, SaveVecNum vecNum)
    {
        DirectX::XMVECTOR vec = DirectX::XMVectorZero();

        vec = DirectX::XMVectorSetX(vec , json[name].value("X" , 0.0f));
        vec = DirectX::XMVectorSetY(vec , json[name].value("Y" , 0.0f));

        if (vecNum >= SaveVecNum::Triple)
            vec = DirectX::XMVectorSetZ(vec, json[name].value("Z", 0.0f));
        if (vecNum >= SaveVecNum::Quad)
            vec = DirectX::XMVectorSetW(vec, json[name].value("W", 0.0f));

        return vec;
    }

    DirectX::XMFLOAT2 LoadJsonToFloat2(const nlohmann::json& json, const std::string_view name)
    {
        DirectX::XMFLOAT2 vec = DirectX::XMFLOAT2(json[name].value("X", 0.0f) , json[name].value("Y", 0.0f));

        return vec;
    }

    DirectX::XMFLOAT3 LoadJsonToFloat3(const nlohmann::json& json, const std::string_view name)
    {
        DirectX::XMFLOAT3 vec = DirectX::XMFLOAT3(json[name].value("X", 0.0f), json[name].value("Y", 0.0f) , json[name].value("Z", 0.0f));

        return vec;
    }

    DirectX::XMFLOAT4 LoadJsonToFloat4(const nlohmann::json& json, const std::string_view name)
    {
        DirectX::XMFLOAT4 vec = DirectX::XMFLOAT4(json[name].value("X", 0.0f), json[name].value("Y", 0.0f), json[name].value("Z", 0.0f), json[name].value("W", 0.0f));

        return vec;
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

    UINT Align256(UINT size)
    {
        return (size + 255) & ~255;
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
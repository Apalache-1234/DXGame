#include "PCH/pch.h"
#include "PhysicsSystem.h"
#include "json.hpp"
#include <fstream>

PhysicsSystem::PhysicsSystem()
{
}

PhysicsSystem::~PhysicsSystem()
{
}

namespace
{
    const std::string folderName = "PhysicsSystemData";
    const std::string jsonName = "GravityData.json";
}

void PhysicsSystem::LoadJson(const std::string scenePath)
{
    nlohmann::json data;

    std::string path = scenePath + "/" + folderName + "/" + jsonName;

    std::ifstream jsonData(path);

    if (jsonData.is_open())
    {
        jsonData >> data;
        jsonData.close();

        gravityPow = data.value("GravityPow", 0.0f);
        SetGravityVec(Utility::LoadJsonToVec(data, "GravityVec"));
    }
}

void PhysicsSystem::SaveJson(const std::string scenePath)
{
    nlohmann::json data;

    data["GravityPow"] = gravityPow;
    Utility::SaveVectorToJson(data, "GravityVec", GetGravityVec(), Utility::SaveVecNum::Triple);

    std::string path = scenePath + "/" + folderName;

    filesystem::create_directories(path);

    path = path + "/" + jsonName;

    std::ofstream jsonData(path);

    jsonData << data.dump(4);
    jsonData.close();
}

void PhysicsSystem::SetGravityVec(DirectX::XMVECTOR vec)
{
    gravityVec = vec;
}

DirectX::XMVECTOR PhysicsSystem::GetGravityVec()
{
    return  gravityVec;
}

DirectX::XMVECTOR PhysicsSystem::GetGravity()
{
    return DirectX::XMVectorScale(DirectX::XMVector3Normalize(gravityVec) , gravityPow);
}

void PhysicsSystem::MainLoop()
{
    auto list = SystemManager::GetInstance().GetComponentList<Rigidbody>();

    for (auto comp : list)
    {
        if (comp->isStatic)
            continue;

        comp->AddForce(DirectX::XMVectorScale(GetGravity() , comp->GetMass() * SystemManager::GetInstance().GetGameDeltaTime()));

        comp->attachFrom.lock()->AddWorldPosition(DirectX::XMVectorScale(comp->GetVelocity(), SystemManager::GetInstance().GetGameDeltaTime()));
    }
}

std::unique_ptr<PhysicsSystem> PhysicsSystem::instance = nullptr;

PhysicsSystem& PhysicsSystem::GetInstance()
{
    if (!instance) {
        instance.reset(new PhysicsSystem());
    }
    return *instance;
}

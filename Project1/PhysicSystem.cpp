#include "PhysicSystem.h"

PhysicSystem::PhysicSystem()
{
}

PhysicSystem::~PhysicSystem()
{
}

void PhysicSystem::SetGravityVec(DirectX::XMVECTOR vec)
{
    gravityVec = vec;
}

DirectX::XMVECTOR PhysicSystem::GetGravityVec()
{
    return  gravityVec;
}

DirectX::XMVECTOR PhysicSystem::GetGravity()
{
    return DirectX::XMVectorScale(DirectX::XMVector3Normalize(gravityVec) , gravityPow);
}

void PhysicSystem::MainLoop()
{
    auto list = GameManager::GetInstance().GetComponentList<Rigidbody>();

    for (auto comp : list)
    {
        if (comp->isStatic)
            continue;

        comp->AddForce(DirectX::XMVectorScale(GetGravity() , comp->GetMass() * GameManager::GetInstance().GetGameDeltaTime()));

        comp->attachFrom->AddWorldPosition(DirectX::XMVectorScale(comp->GetVelocity(), GameManager::GetInstance().GetGameDeltaTime()));
    }
}

std::unique_ptr<PhysicSystem> PhysicSystem::instance = nullptr;

PhysicSystem& PhysicSystem::GetInstance()
{
    if (!instance) {
        instance.reset(new PhysicSystem());
    }
    return *instance;
}

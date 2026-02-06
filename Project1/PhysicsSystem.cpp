#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
}

PhysicsSystem::~PhysicsSystem()
{
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

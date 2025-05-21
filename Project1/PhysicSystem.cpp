#include "PhysicSystem.h"

PhysicSystem::PhysicSystem()
{
}

PhysicSystem::~PhysicSystem()
{
}

void PhysicSystem::SetGravityVec(DirectX::XMVECTOR vec)
{
    gravityVec = DirectX::XMVector3Normalize(vec);
}

DirectX::XMVECTOR PhysicSystem::GetGravity()
{
    return DirectX::XMVectorScale(gravityVec , gravityPow);
}

void PhysicSystem::MainLoop()
{
    auto list = GameManager::GetInstance().GetComponentList<Rigidbody>();

    for (auto comp : list)
    {
        if (comp->isStatic)
            continue;

        comp->AddForce(DirectX::XMVectorScale(GetGravity() , comp->GetMass()));

        comp->attachFrom->AddWorldPosition(DirectX::XMVectorScale(comp->GetVelocity(), GameManager::GetInstance().DeltaTime));
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

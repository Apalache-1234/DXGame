#ifndef PHYSIC_SYSTEM_H
#define PHYSIC_SYSTEM_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "GameManager.h"
#include "Entity.h"
#include "Collider.h"
#include "Utility.h"
#include "Rigidbody.h"

class PhysicSystem
{
public:
	PhysicSystem();

	~PhysicSystem();

	float gravityPow = 0.7;

	void SetGravityVec(DirectX::XMVECTOR vec);

	DirectX::XMVECTOR GetGravity();

	void MainLoop();

	static PhysicSystem& GetInstance();
private:
	/// <summary>
    /// ゲーム上のあらゆる情報を確保するGameManagerクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<PhysicSystem> instance;

	DirectX::XMVECTOR gravityVec = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0, -1, 0, 0));
};
#endif
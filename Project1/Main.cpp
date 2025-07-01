#include "Window.h"
#include "DXRenderSystem.h"

#include <iostream>
#include <DirectXMath.h>
#include "entity.h"
#include "Utility.h"
#include "MyIMGUI.h"
#include "ColliderDetection.h"
#include "Collider.h"
#include "Camera.h"
#include "PhysicSystem.h"
#include "Rigidbody.h"
#include "SkyBox.h"
#include "UIRenderer.h"
#include "MeshLoader.h"
#include "SkinnedRenderer.h"
#include "DebugCollider.h"
#include "DebugOrigin.h"
#include "KeyInput.h"
#include "DebugRayCast.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "DebugPointLight.h"
#include "DebugDL.h"
#include "DebugSpotLight.h"
#include "DebugGrids.h"

float hoge = 0;

void GameMainLoop()
{

}

int main()
{
	auto window = &Window::GetInstance();
	auto renSys = &DXRenderSystem::GetInstance();
	auto myImGui = &MyIMGUI::GetInstance();
	auto gameManager = &GameManager::GetInstance();
	auto physicSystem = &PhysicSystem::GetInstance();
	auto keySystem = &KeyInput::GetInstance();


	window->Init();
	renSys->Init();
	if (myImGui->useIMGUI)
		myImGui->Init();

	auto meshLoader = &MeshLoader::GetInstance();

	meshLoader->Init();

	auto player = gameManager->CreateEntity("Player");
	auto player4 = gameManager->CreateEntity("Player4");
	auto player5 = gameManager->CreateEntity("PointLight");
	auto player2 = gameManager->CreateEntity("Player2");
	auto Skybox = gameManager->CreateEntity("Skybox");
	auto debugColCube = gameManager->CreateEntity("DebugCube");
	auto debugColSphere = gameManager->CreateEntity("DebugSphere");
	auto debugOrigin = gameManager->CreateEntity("DebugOrigin");
	auto debugRay = gameManager->CreateEntity("DebugRay");
	auto debugDL = gameManager->CreateEntity("DebugDirectionalLight");
	auto debugPL = gameManager->CreateEntity("DebugPointLight");
	auto debugSL = gameManager->CreateEntity("DebugSpotLight");
	auto debugGrids = gameManager->CreateEntity("DebugGrids");

	player5->AddComponent<SpotLight>();

	player->AddComponent<Collider>();
	player->AddComponent<Rigidbody>();

	player->GetComponent<Rigidbody>()->isStatic = true;

	player->SetWorldScale(DirectX::XMVectorSet(100,10,100,0));
	player->SetWorldPosition(DirectX::XMVectorSet(0,-5,0,0));
	player2->SetWorldScale(DirectX::XMVectorSet(1, 1, 1,0));
	player2->SetWorldPosition(DirectX::XMVectorSet(0,10,0,0));
	player2->SetWorldRotation(DirectX::XMVectorSet(0,0,0,0));

	Skybox->RemoveComponent<SkinnedRenderer>();
	Skybox->AddComponent<SkyBox>();

	debugColCube->RemoveComponent<SkinnedRenderer>();
	debugColCube->AddComponent<DebugCollider>();

	debugColSphere->RemoveComponent<SkinnedRenderer>();
	debugColSphere->AddComponent<DebugCollider>(Collider::ColliderType::Sphere);

	debugOrigin->RemoveComponent<SkinnedRenderer>();
	debugOrigin->AddComponent<DebugOrigin>();

	debugRay->RemoveComponent<SkinnedRenderer>();
	debugRay->AddComponent<DebugRayCast>();

	debugDL->RemoveComponent<SkinnedRenderer>();
	debugDL->AddComponent<DebugDirectionalLight>();

	debugPL->RemoveComponent<SkinnedRenderer>();
	debugPL->AddComponent<DebugPointLight>();

	debugSL->RemoveComponent<SkinnedRenderer>();
	debugSL->AddComponent<DebugSpotLight>();

	debugGrids->RemoveComponent<SkinnedRenderer>();
	debugGrids->AddComponent<DebugGrids>();

	bool show_demo_window = true;

	UINT fpsCounter = 0;

	float timeForFpsCounter = 0;

	while (window->MainLoop())
	{
		gameManager->MainLoop();
		physicSystem->MainLoop();
		keySystem->Update();

		hoge += 360 * gameManager->DeltaTime;

		DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);

		if (keySystem->GetKeyDown(A))
		{
			velocity = DirectX::XMVectorAdd(velocity , Utility::left);
		}

		if (keySystem->GetKeyDown(D))
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::right);
		}

		if (keySystem->GetKeyDown(W))
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::forward);
		}

		if (keySystem->GetKeyDown(S))
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::backward);
		}

		if (keySystem->GetKey(SPACE))
		{
			player2->GetComponent<Rigidbody>()->SetVelocity(DirectX::XMVectorScale(Utility::up , 20));
		}

		RayCast ray = RayCast(player4->GetWorldPosition() , player4->GetForward(), 3);

		if (keySystem->GetKey(Z))
		{
			player4->AddWorldPosition(DirectX::XMVectorScale(player4->GetForward(), 0.1));
		}

		velocity = DirectX::XMVectorScale(velocity , 0.07);

		auto hoge1 = DirectX::XMVectorScale(player2->GetRight(),4);
		auto hoge2 = DirectX::XMVectorScale(player2->GetUp(),4);
		auto hoge3 = DirectX::XMVectorScale(player2->GetForward(),4);

		auto vec = player2->GetWorldPosition();

		auto axis = DirectX::XMVectorSet(0.5, 0.5, 0.5, 0);

		axis = DirectX::XMVector3Normalize(axis);

		auto mousePos = keySystem->GetMousePos();

		auto norVec = DirectX::XMVectorSubtract(Camera::GetInstance().GetPosScreenToWorld(DirectX::XMLoadFloat2(&mousePos)), Camera::GetInstance().GetPosition());

		RayCast(Camera::GetInstance().GetPosition(), DirectX::XMVector3Normalize(norVec), 100.0f);

		if (keySystem->GetKeyDown(T))
		{
			player4->SetQuaternion(DirectX::XMQuaternionRotationAxis(axis, 0.01));
		}

		hoge1 = DirectX::XMVectorAdd(hoge1 , vec);
		hoge2 = DirectX::XMVectorAdd(hoge2 , vec);
		hoge3 = DirectX::XMVectorAdd(hoge3 , vec);

		ColliderDetection::GetInstance().MainLoop();

		renSys->Draw();

		timeForFpsCounter += gameManager->DeltaTime;

		fpsCounter++;

		if (timeForFpsCounter >= 1)
		{
			gameManager->GetInstance().fps = fpsCounter;

			fpsCounter = 0;

			timeForFpsCounter = 0;
		}
	}

	myImGui->Term();

	return 0;
}



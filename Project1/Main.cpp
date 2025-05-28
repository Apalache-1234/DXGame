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
#include "DebugColliderCube.h"

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


	window->Init();
	renSys->Init();
	if (myImGui->useIMGUI)
		myImGui->Init();

	auto meshLoader = &MeshLoader::GetInstance();

	meshLoader->Init();

	auto player = gameManager->CreateEntity("Player");
	auto player4 = gameManager->CreateEntity("Player4");
	auto player2 = gameManager->CreateEntity("Player2");
	auto Skybox = gameManager->CreateEntity("Skybox");
	auto debugCol = gameManager->CreateEntity("Debug");

	player2->AddComponent<Collider>();
	player2->AddComponent<Rigidbody>();

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

	debugCol->RemoveComponent<SkinnedRenderer>();
	debugCol->AddComponent<DebugColiderCube>();

	bool show_demo_window = true;

	UINT fpsCounter = 0;

	float timeForFpsCounter = 0;

	while (window->MainLoop())
	{
		gameManager->MainLoop();
		physicSystem->MainLoop();

		hoge += 10;

		DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);

		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity , Utility::left);
		}

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::right);
		}

		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::up);
		}

		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::down);
		}

		if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::forward);
		}

		if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::backward);
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			player2->GetComponent<Rigidbody>()->SetVelocity(DirectX::XMVectorScale(Utility::up , 6));
		}

		velocity = DirectX::XMVectorScale(velocity , 0.07);

		player2->GetComponent<Rigidbody>()->AddForce(velocity);

		auto hoge1 = DirectX::XMVectorScale(player2->GetRight(),4);
		auto hoge2 = DirectX::XMVectorScale(player2->GetUp(),4);
		auto hoge3 = DirectX::XMVectorScale(player2->GetForward(),4);

		auto vec = player2->GetWorldPosition();

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



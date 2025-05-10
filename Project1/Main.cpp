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
	auto skyBox = &SkyBox::GetInstance();

	window->Init();
	renSys->Init();
	if (myImGui->useIMGUI)
		myImGui->Init();
	skyBox->Init();


	shared_ptr<Entity> player = make_shared<Entity>("Player");
	shared_ptr<Entity> player2 = make_shared<Entity>("Player2");

	player2->AddComponent<Renderer>();
	player2->AddComponent<Collider>();
	player2->AddComponent<Rigidbody>();
	player->AddComponent<Renderer>();
	player->AddComponent<Collider>();
	player->AddComponent<Rigidbody>();

	player->GetComponent<Rigidbody>()->isStatic = true;

	player->SetWorldScale(DirectX::XMVectorSet(100,10,100,0));
	player->SetWorldPosition(DirectX::XMVectorSet(0,-5,0,0));
	player2->SetWorldScale(DirectX::XMVectorSet(0.01, 0.01, 0.01,0));

	player2->SetWorldPosition(DirectX::XMVectorSet(0,10,0,0));
	player2->SetWorldRotation(DirectX::XMVectorSet(0,0,0,0));

	bool show_demo_window = true;

	player2->GetComponent<Renderer>()->LoadMesh("Models/Knight01/Animated_Low_Poly_Dark_Knight_BAKED.fbx");

	UINT fpsCounter = 0;

	float timeForFpsCounter = 0;

	while (window->MainLoop())
	{
		gameManager->MainLoop();
		physicSystem->MainLoop();

		if (myImGui->useIMGUI)
			myImGui->MainLoop();

		hoge += 10;

		auto rend = player2->GetComponent<Renderer>();


		DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);

		Camera::GetInstance().SetCameraRotation(DirectX::XMVectorSet(0, hoge * 0.01, 0, 0));
		

		rend->StartAnimation(gameManager->GameTime);

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

		player2->GetComponent<Rigidbody>()->AddVelocity(velocity);

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
			std::cout << fpsCounter << std::endl;

			fpsCounter = 0;

			timeForFpsCounter = 0;
		}
	}

	myImGui->Term();

	return 0;
}



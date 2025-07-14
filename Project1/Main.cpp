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
#include "PhysicsSystem.h"
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
#include "DebugUIRenderer.h"
#include "TweenSystem.h"
#include "Tween.h"

float hoge = 0;

void GameMainLoop()
{

}

int main()
{
	auto window = &Window::GetInstance();
	auto renSys = &DXRenderSystem::GetInstance();
	auto myImGui = &MyIMGUI::GetInstance();
	auto SystemManager = &SystemManager::GetInstance();
	auto PhysicsSystem = &PhysicsSystem::GetInstance();
	auto keySystem = &KeyInput::GetInstance();
	auto tweenSystem = &TweenSystem::GetInstance();


	window->Init();
	renSys->Init();
	if (myImGui->useIMGUI)
		myImGui->Init();

	auto meshLoader = &MeshLoader::GetInstance();

	meshLoader->Init();

	auto player = SystemManager->CreateEntity<SkinnedRenderer>("Player");
	auto player4 = SystemManager->CreateEntity<SkinnedRenderer>("Player4");
	auto player5 = SystemManager->CreateEntity<SpotLight>("PointLight");
	auto player2 = SystemManager->CreateEntity<SkinnedRenderer>("Player2");
	auto Skybox = SystemManager->CreateEntity<SkyBox>("Skybox");
	auto debugColCube = SystemManager->CreateEntity<DebugCollider>("DebugCube");
	auto debugColSphere = SystemManager->CreateEntity("DebugSphere");
	auto debugOrigin = SystemManager->CreateEntity<DebugOrigin>("DebugOrigin");
	auto debugRay = SystemManager->CreateEntity<DebugRayCast>("DebugRay");
	auto debugDL = SystemManager->CreateEntity<DebugDirectionalLight>("DebugDirectionalLight");
	auto debugPL = SystemManager->CreateEntity<DebugPointLight>("DebugPointLight");
	auto debugSL = SystemManager->CreateEntity<DebugSpotLight>("DebugSpotLight");
	auto debugGrids = SystemManager->CreateEntity<DebugGrids>("DebugGrids");
	auto debugUI = SystemManager->CreateEntity<DebugUIRenderer>("DebugUIRenderer");

	player->AddComponent<Collider>();
	player->AddComponent<Rigidbody>();

	player->GetComponent<Rigidbody>()->isStatic = true;

	player->SetWorldScale(DirectX::XMVectorSet(100,10,100,0));
	player->SetWorldPosition(DirectX::XMVectorSet(0,-5,0,0));
	player2->SetWorldScale(DirectX::XMVectorSet(1, 1, 1,0));
	player2->SetWorldPosition(DirectX::XMVectorSet(0,10,0,0));
	player2->SetWorldRotation(DirectX::XMVectorSet(0,0,0,0));

	debugColSphere->AddComponent<DebugCollider>(Collider::ColliderType::Sphere);

	bool show_demo_window = true;

	UINT fpsCounter = 0;

	float timeForFpsCounter = 0;

	shared_ptr<Tween<XMVECTOR>> tw = nullptr;

	while (window->MainLoop())
	{
		SystemManager->MainLoop();
		PhysicsSystem->MainLoop();
		keySystem->Update();
		tweenSystem->Update();

		hoge += 360 * SystemManager->GetGameDeltaTime();

		DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);

		if (keySystem->GetKey(I))
		{
			tw = tweenSystem->DoTween<XMVECTOR>(DirectX::XMVectorSet(0,10 , 0 , 0), DirectX::XMVectorZero(), EasingType::EaseOutQuint);

			tw->OnComplete([]() { std::cout << "COMP" << std::endl; return; });
			tw->OnStart([]() { std::cout << "START" << std::endl; return; });
			tw->OnUpdate([]() { std::cout << "UPDATE" << std::endl; return; });
		}

		if (keySystem->GetKeyDown(N))
		{
			tw = nullptr;
		}

		if (tw)
		{
			player->SetWorldPosition(tw->GetValue());
		}

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

		timeForFpsCounter += SystemManager->GetGameDeltaTime();

		fpsCounter++;

		if (timeForFpsCounter >= 1)
		{
			SystemManager->GetInstance().fps = fpsCounter;

			fpsCounter = 0;

			timeForFpsCounter = 0;
		}
	}

	myImGui->Term();

	return 0;
}



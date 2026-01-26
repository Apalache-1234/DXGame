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
#include "GameScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

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
	auto SceneManager = &SceneManager::GetInstance();

	window->Init();
	renSys->Init();
	if (myImGui->useIMGUI)
		myImGui->Init();

	auto meshLoader = &MeshLoader::GetInstance();

	meshLoader->Init();


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

	debugColSphere->AddComponent<DebugCollider>(Collider::ColliderType::Sphere);

	SceneManager->AddScene(std::make_shared<TitleScene>("TitleScene"));

	shared_ptr<Tween<XMVECTOR>> tw = nullptr;

	while (window->MainLoop())
	{
		SystemManager->MainLoop();
		PhysicsSystem->MainLoop();
		keySystem->Update();
		tweenSystem->Update();

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

		ColliderDetection::GetInstance().MainLoop();

		renSys->Draw();
	}

	myImGui->Term();

	return 0;
}



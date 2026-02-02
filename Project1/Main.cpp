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
#include "CardGameScene.h"

float hoge = 0;

void GameMainLoop()
{

}

int main()
{
	auto window = &Window::GetInstance();
	auto renSys = &DXRenderSystem::GetInstance();
	auto myImGui = &MyIMGUI::GetInstance();
	auto PhysicsSystem = &PhysicsSystem::GetInstance();
	auto keySystem = &KeyInput::GetInstance();
	auto SceneManager = &SceneManager::GetInstance();

	window->Init();
	renSys->Init();

	if (myImGui->useIMGUI)
		myImGui->Init();

	auto meshLoader = &MeshLoader::GetInstance();

	meshLoader->Init();

	auto SystemManager = &SystemManager::GetInstance();
	auto tweenSystem = &TweenSystem::GetInstance();

	auto Skybox = SystemManager->CreateEntity<SkyBox>("Skybox");
	auto debugColCube = SystemManager->CreateEntity<DebugCollider>("DebugCube");
	auto debugColSphere = SystemManager->CreateEntity("DebugSphere");
	auto debugOrigin = SystemManager->CreateEntity<DebugOrigin>("DebugOrigin");
	auto debugRay = SystemManager->CreateEntity<DebugRayCast>("DebugRay");
	auto debugDL = SystemManager->CreateEntity<DebugDirectionalLight>("DebugDirectionalLight");
	auto debugPL = SystemManager->CreateEntity<DebugPointLight>("DebugPointLight");
	auto debugSL = SystemManager->CreateEntity<DebugSpotLight>("DebugSpotLight");
	auto debugUI = SystemManager->CreateEntity<DebugUIRenderer>("DebugUIRenderer");
	auto debugGrids = SystemManager->CreateEntity<DebugGrids>("DebugGrids");

	debugColSphere->AddComponent<DebugCollider>(Collider::ColliderType::Sphere);

	SceneManager->AddScene(std::make_shared<TitleScene>("TitleScene"));
	SceneManager->AddScene(std::make_shared<CardGameScene>("CardGameScene"));

	shared_ptr<Tween<XMVECTOR>> tw = nullptr;

	while (window->MainLoop())
	{
		SystemManager->MainLoop();
		PhysicsSystem->MainLoop();
		keySystem->Update();
		tweenSystem->Update();

		if (keySystem->GetKey(D))
		{
			SceneManager->ChangeGameScene("TitleScene");
		}

		if (keySystem->GetKey(A))
		{
			SceneManager->ChangeGameScene("CardGameScene");
		}

		if (keySystem->GetKeyDown(N))
		{
			tw = nullptr;
		}

		ColliderDetection::GetInstance().MainLoop();

		renSys->Draw();

		if (SystemManager->GetIsExit())
		{
			window->Close();
		}
	}

	renSys->WaitForRender();

	myImGui->Term();

	return 0;
}



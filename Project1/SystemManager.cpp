#include "SystemManager.h"
#include "Flags.h"
#include "ScriptComponent.h"
#include "DXRenderSystem.h"
#include "Flags.h"
#include "PEEdgeFilter.h"
#include "PEHorizonGaussianBlur.h"
#include "PEVerticalGaussianBlur.h"
#include "PEMosaicFilter.h"
#include "PERadicalBlur.h"

SystemManager::SystemManager()
{
	QueryPerformanceCounter(&base);
	QueryPerformanceCounter(&baseReal);
	QueryPerformanceFrequency(&freq);
}

SystemManager::~SystemManager()
{
}

std::unique_ptr<SystemManager> SystemManager::instance = nullptr;

SystemManager& SystemManager::GetInstance()
{
	if (!instance) {
		instance.reset(new SystemManager());
	}
	return *instance;
}

std::vector<std::shared_ptr<Entity>>& SystemManager::GetEntityList()
{
	return entities;
}

bool SystemManager::isExitNameEntity(std::string name)
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->GetName() == name)
			return true;
	}

	return false;
}

void SystemManager::SetEntity(std::shared_ptr<Entity> entity)
{
	entities.emplace_back(entity);
	entitiesN[entity->GetName()] = entity;
}

std::shared_ptr<Entity> SystemManager::GetEntityByName(std::string name)
{
	if (entitiesN.count(name) > 0)
		return entitiesN[name];

	std::cout << "entity : " << name << " はそんざいしません" << std::endl;

	return nullptr;
}

std::shared_ptr<Entity> SystemManager::CreateEntity(std::string name)
{
	if (isExitNameEntity(name))
	{
		int i = 1;

		while (true)
		{
			std::string nameNum = name + std::to_string(i);

			if (!isExitNameEntity(nameNum))
			{
				name = nameNum;
				break;
			}

			i++;
		}
	}

	auto entity = std::make_shared<Entity>(name);

	entity->Init();

	SetEntity(entity);

	return entity;
}

void SystemManager::UpdateDestroyedEntities()
{
	for (int i = 0;i < entities.size();i++)
	{
		if (entities[i]->IsDestoroyed())
		{
			entitiesN.erase(entities[i]->GetName());
			entities.erase(entities.begin() + i);
		}
		else
		{
			auto scripts = entities[i]->GetComponents<ScriptComponent>();

			for (auto script : scripts)
			{
				script->OnUpdate();
			}

			while (fixedUpdateCounter > sixtyDenom)
			{
				for (auto script : scripts)
				{
					script->OnFixedUpdate();
				}

				fixedUpdateCounter -= sixtyDenom;
			}
		}
	}
}

void SystemManager::MainLoop()
{
	if (Flags::GetInstance().isTimeStop && !pause)
	{
		PauseTime();
	}
	else if(!Flags::GetInstance().isTimeStop && pause)
	{
		UnpauseTime();
	}

	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);

	RealTime = (float)(curr.QuadPart - baseReal.QuadPart) / freq.QuadPart;

	RealDeltaTime = RealTime - prevRealTime;

	prevRealTime = RealTime;

	if (pause)
	{
		GameDeltaTime = 0;
		return;
	}
		
	GameTime = (float)(curr.QuadPart - base.QuadPart) / freq.QuadPart;

	GameDeltaTime = GameTime - prevGameTime;

	prevGameTime = GameTime;

	fixedUpdateCounter += RealDeltaTime;

	UpdateDestroyedEntities();

	timeForFpsCounter += GetRealDeltaTime();

	fpsCounter++;

	if (timeForFpsCounter >= 1)
	{
		fps = fpsCounter;

		std::cout << fps << std::endl;

		fpsCounter = 0;

		timeForFpsCounter = 0;
	}
}

void SystemManager::PauseTime()
{
	pause = true;

	QueryPerformanceCounter(&pauseClocks);

	std::cout << "ポーズ中" << std::endl;
}

void SystemManager::UnpauseTime()
{
	pause = false;

	QueryPerformanceCounter(&unpauseClocks);

	auto sumClocksWhilePause = unpauseClocks.QuadPart - pauseClocks.QuadPart;

	base.QuadPart += sumClocksWhilePause;

	std::cout << "ポーズ解除" << std::endl;
}

bool SystemManager::GetIsPause()
{
	return pause;
}

float SystemManager::GetGameTime()
{
	return GameTime;
}

float SystemManager::GetGameDeltaTime()
{
	return GameDeltaTime;
}

float SystemManager::GetRealTime()
{
	return RealTime;
}

float SystemManager::GetRealDeltaTime()
{
	return RealDeltaTime;
}

void SystemManager::AllReset()
{
	UpdateDestroyedEntities();

	DXRenderSystem* dxrs = &DXRenderSystem::GetInstance();
	Flags* flags = &Flags::GetInstance();

	dxrs->fogFlag = false;
	dxrs->SetFogColor(DirectX::XMVectorSplatOne());
	dxrs->SetFogRange(1000);
	dxrs->SetCascadeShadowLerp(0.9f);
	dxrs->SetCascadeShadowNearZ(0.1f);
	dxrs->SetCascadeShadowFarZ(1000);
	dxrs->SetCascadeShadowCount(4);
	dxrs->SetAmbientColor(DirectX::XMFLOAT3(1,1,1));
	
	auto posts = GetComponentList<PostEffect>();

	for (auto& post : posts)
	{
		post->flag = false;
	}

	Camera* camera = &Camera::GetInstance();

	camera->SetFov(DirectX::XMConvertToRadians(70));
	camera->SetCameraPosition(DirectX::XMVectorSet(0, 2, -4, 0));
	camera->SetCameraRotation(DirectX::XMVectorSet(0, 0, 10, 0));
	camera->SetFarZ(1000);
	camera->SetNearZ(0.01f);
	camera->SetOrthoDiameter(10);

	flags->isTimeStop = false;

}

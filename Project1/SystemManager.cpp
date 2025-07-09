#include "SystemManager.h"
#include "Flags.h"

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

void SystemManager::MainLoop()
{
	if (*Flags::GetInstance().isTimeStop && !pause)
	{
		PauseTime();
	}
	else if(!*Flags::GetInstance().isTimeStop && pause)
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

	for (int i = 0;i<entities.size();i++)
	{
		if (entities[i]->IsDestoroyed())
		{
			entities.erase(entities.begin() + i);
		}
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

DirectX::XMFLOAT3 SystemManager::GetAmbientColor()
{
	return ambientColor;
}

void SystemManager::SetAmbientColor(DirectX::XMFLOAT3 col)
{
	auto xmvec = DirectX::XMVectorClamp(DirectX::XMLoadFloat3(&col), DirectX::XMVectorZero(), DirectX::XMVectorSplatOne());

	DirectX::XMStoreFloat3(&ambientColor , xmvec);
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

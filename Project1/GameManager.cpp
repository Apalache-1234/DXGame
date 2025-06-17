#include "GameManager.h"
#include "Flags.h"

GameManager::GameManager()
{
	QueryPerformanceCounter(&base);
	QueryPerformanceFrequency(&freq);
}

GameManager::~GameManager()
{
}

std::unique_ptr<GameManager> GameManager::instance = nullptr;

GameManager& GameManager::GetInstance()
{
	if (!instance) {
		instance.reset(new GameManager());
	}
	return *instance;
}

std::vector<std::shared_ptr<Entity>>& GameManager::GetEntityList()
{
	return entities;
}

bool GameManager::isExitNameEntity(std::string name)
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->GetName() == name)
			return true;
	}

	return false;
}

void GameManager::SetEntity(std::shared_ptr<Entity> entity)
{
	entities.emplace_back(entity);
}


std::shared_ptr<Entity> GameManager::CreateEntity(std::string name)
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

void GameManager::MainLoop()
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

	GameTime = (float)(curr.QuadPart - base.QuadPart) / freq.QuadPart;

	DeltaTime = GameTime - prevTime;

	prevTime = GameTime;

	for (int i = 0;i<entities.size();i++)
	{
		if (entities[i]->IsDestoroyed())
		{
			entities.erase(entities.begin() + i);
		}
	}
}

void GameManager::PauseTime()
{
	pause = true;

	QueryPerformanceCounter(&pauseClocks);

	std::cout << "ポーズ中" << std::endl;
}

void GameManager::UnpauseTime()
{
	pause = false;

	QueryPerformanceCounter(&unpauseClocks);

	auto sumClocksWhilePause = unpauseClocks.QuadPart - pauseClocks.QuadPart;

	base.QuadPart += sumClocksWhilePause;

	std::cout << "ポーズ解除" << std::endl;
}

DirectX::XMFLOAT3 GameManager::GetAmbientColor()
{
	return ambientColor;
}

void GameManager::SetAmbientColor(DirectX::XMFLOAT3 col)
{
	auto xmvec = DirectX::XMVectorClamp(DirectX::XMLoadFloat3(&col), DirectX::XMVectorZero(), DirectX::XMVectorSplatOne());

	DirectX::XMStoreFloat3(&ambientColor , xmvec);
}

#include "GameManager.h"

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

void GameManager::SetEntity(std::shared_ptr<Entity> entity)
{
	entities.emplace_back(entity);
}


void GameManager::MainLoop()
{
	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);

	GameTime = (float)(curr.QuadPart - base.QuadPart) / freq.QuadPart;

	DeltaTime = GameTime - prevTime;

	prevTime = GameTime;
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

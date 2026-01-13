#include "SceneManager.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

std::unique_ptr<SceneManager> SceneManager::instance = nullptr;

SceneManager& SceneManager::GetInstance()
{
    if (!instance) {
        instance.reset(new SceneManager());
    }
    return *instance;
}

void SceneManager::AddScene(GameScene scene)
{
    scenes.emplace_back(std::make_shared<GameScene>(scene));
}

std::shared_ptr<GameScene> SceneManager::GetScene(std::string name)
{
    int index = 1;

    for (int i = 0;i<scenes.size();i++)
    {
        if (scenes[i]->GetName() == name)
            name = name + std::to_string(index);
    }

    return std::shared_ptr<GameScene>();
}

std::shared_ptr<GameScene> SceneManager::GetScene(UINT ind)
{
    return std::shared_ptr<GameScene>();
}

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

std::shared_ptr<GameScene> SceneManager::GetCurrentGameScene()
{
    return currentGameScene;
}

void SceneManager::ChangeGameScene(int ind)
{
    auto scene = GetScene(ind);

    if (nullptr != scene)
    {
        ResetGameScene(scene);
    }
}

void SceneManager::ChangeGameScene(std::string name)
{
    auto scene = GetScene(name);

    if (nullptr != scene)
    {
        ResetGameScene(scene);
    }
}

void SceneManager::ResetGameScene(std::shared_ptr<GameScene> scene)
{
    currentGameScene->Reset();



    currentGameScene = scene;
}

void SceneManager::AddScene(std::shared_ptr<GameScene> scene)
{
    int index = 1;

    std::string tempName = scene->GetName();

    for (int i = 0; i < scenes.size(); i++)
    {
        if (scenes[i]->GetName() == tempName)
            tempName = scene->GetName() + std::to_string(index);
    }

    scenes.emplace_back(std::dynamic_pointer_cast<GameScene>(scene));
    scenesN[scenes.back()->GetName()] = scenes.back();

    if (scenes.size() == 1)
    {
        currentGameScene = scenes.back();

        currentGameScene->Load();
    }
}

std::shared_ptr<GameScene> SceneManager::GetScene(std::string name)
{
    if (scenesN.count(name) > 0)
    {
        return scenesN[name];
    }

    std::cout << "Scene : " << name << " ‚Í‘¶Ý‚µ‚Ü‚¹‚ñ" << std::endl;

    return nullptr;
}

std::shared_ptr<GameScene> SceneManager::GetScene(UINT ind)
{
    if (scenes.size() > ind)
    {
        return scenes[ind];
    }

    std::cout << "Scene : Index " << ind << " ‚Í‘¶Ý‚µ‚Ü‚¹‚ñ" << std::endl;

    return nullptr;
}

#include "PCH/pch.h"
#include "SceneManager.h"
#include "Flags.h"
#include <filesystem>
#include <fstream>
#include "Camera.h"
#include "SkinnedRenderer.h"
#include "PhysicsSystem.h"
#include "json.hpp"

namespace
{
    const std::string folderName = "EntitiesData";
    const std::string jsonName = "ListData.json";
    const std::string keyName = "Entity";
}

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

void SceneManager::Update()
{
    currentGameScene->Update();
}

void SceneManager::SaveEntitiesJson(const std::string scenePath)
{
    nlohmann::json data;

    std::string path = scenePath + "/" + folderName;

    filesystem::create_directories(path);

    auto list = GetCurrentGameScene()->GetEntities();

    for (int i = 0; i < list.size(); i++)
    {
        data[keyName][i] = list[i]->GetName();
        list[i]->SaveJson(path);
    }

    path = path + "/" + jsonName;

    std::ofstream jsonData(path);

    jsonData << data.dump(4);
    jsonData.close();
}

void SceneManager::LoadEntitiesJson(const std::string scenePath)
{
    nlohmann::json data;

    std::string folderPath = scenePath + "/" + folderName;
    std::string jsonPath = folderPath + "/" + jsonName;

    std::ifstream jsonData(jsonPath);

    if (jsonData.is_open())
    {
        jsonData >> data;
        jsonData.close();

        if (data.contains(keyName))
        {
            for (auto& name : data[keyName].items())
            {
                const auto& ent = SceneManager::GetInstance().GetCurrentGameScene()->CreateEntity(name.value());
            }
        }

        for (auto& ent : SceneManager::GetInstance().GetCurrentGameScene()->GetEntities())
        {
            ent->LoadJson(folderPath);
        }
    }
}

void SceneManager::SaveScene()
{
    string rootPath = "Scenes/";

    std::string name = GetCurrentGameScene()->GetName() + ".scene";

    string path = "";

    path = rootPath + name;

    filesystem::create_directory(path);

    Camera::GetInstance().SaveJson(path);
    DXRenderSystem::GetInstance().SaveJson(path);
    PhysicsSystem::GetInstance().SaveJson(path);
    SaveEntitiesJson(path);
}

void SceneManager::LoadScene()
{
    string rootPath = "Scenes/";

    std::string name = GetCurrentGameScene()->GetName() + ".scene";

    string path = "";

    path = rootPath + name;

    filesystem::create_directory(path);

    Camera::GetInstance().LoadJson(path);
    DXRenderSystem::GetInstance().LoadJson(path);
    PhysicsSystem::GetInstance().LoadJson(path);
    LoadEntitiesJson(path);
}

void SceneManager::ResetGameScene(std::shared_ptr<GameScene> scene)
{
    if (currentGameScene != nullptr)
        currentGameScene->Reset();

    SystemManager::GetInstance().AllReset();

    currentGameScene = scene;

    currentGameScene->Load();

    LoadScene();
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

    SystemManager::GetInstance().Debug("CurScene = " + scenes.back()->GetName());

    if (scenes.size() == 1)
    {
        ResetGameScene(scenes.back());
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

std::vector<std::shared_ptr<GameScene>> SceneManager::GetScenes()
{
    return scenes;
}

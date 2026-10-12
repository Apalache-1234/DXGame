#include "PCH/pch.h"
#include "GameStartingScript.h"
#include "UIRenderer.h"
#include "PEEdgeFilter.h"
#include "ReadyUIScript.h"
#include <filesystem>

namespace fs = std::filesystem;

GameStartingScript::GameStartingScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{

}

GameStartingScript::~GameStartingScript()
{
}

void GameStartingScript::OnStart()
{
	uiRend = attachFrom.lock()->AddComponent<UIRenderer>();

    uiRend->color = DirectX::XMVectorZero();

    attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(2,2,2,1));

    std::string path = "./Textures/FireBall";

    try {
        if (fs::exists(path) && fs::is_directory(path)) {

            for (const auto& entry : fs::directory_iterator(path)) {

                if (entry.is_regular_file()) {
                    file_names.push_back(entry.path().filename().string());
                }
            }
        }
        else {
            std::cerr << "ディレクトリが見つかりません。" << std::endl;
        }

    }
    catch (const fs::filesystem_error& ex) {
        std::cerr << "エラー: " << ex.what() << std::endl;
    }

    for (const auto& name : file_names) {
        textureLoader->LoadTexture("Textures/FireBall/" + name);
    }

    spriteTw = tweenSystem->DoTween(0,(int)file_names.size()-1,EasingType::EaseLine , 2);

    spriteTw->Pause();

    edgePE = systemManager->GetComponentList<PEEdgeFilter>().back();

    edgePE->flag = true;

    edgePE->SetTransition(0);

    readyEnt = sceneManager->GetCurrentGameScene()->CreateEntity<ReadyUIScript>("STARTUI");

    readyRend = readyEnt->GetComponent<UIRenderer>();

    readyRend->CreateTexture("Textures/GAMESTART.png");

    readyRend->color = DirectX::XMVectorSet(1, 1, 1, 0);
}

void GameStartingScript::StartAnim(std::function<void()> f)
{
    animStarted = true;

    spriteTw->Play();

    spriteTw->OnComplete([=]() {f();   edgePE->flag = false; });

    uiRend->CreateTexture("Textures/FireBall/" + file_names[0]);

    uiRend->color = DirectX::XMVectorSplatOne();
}

void GameStartingScript::OnUpdate()
{
    if (!animStarted)
        return;

    uiRend->CreateTexture("Textures/FireBall/" + file_names[spriteTw->GetValue()]);

    uiRend->color = DirectX::XMVectorSet(1,1,1,1.0f - ((float) spriteTw->GetValue() / 100));

    float rad = std::sin(((float)spriteTw->GetValue() / 100) * Utility::PI);

    edgePE->SetTransition(rad);

    readyRend->color = DirectX::XMVectorSet(rad,rad,rad,rad);
}

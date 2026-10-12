#include "PCH/pch.h"
#include "TitleFrameScript.h"
#include "TitleSpotLightScript.h"
#include "GameStartUIScript.h"
#include "Collider.h"

bool TitleFrameScript::isPressed = false;

TitleFrameScript::TitleFrameScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TitleFrameScript::~TitleFrameScript()
{
}

void TitleFrameScript::OnStart()
{
	rend = attachFrom.lock()->AddComponent<SkinnedRenderer>();
	col = attachFrom.lock()->AddComponent<Collider>();

	col->scale = DirectX::XMVectorSet(0.03,0.08 , 0.13,0);
	col->position = DirectX::XMVectorSet(-0.01,0 , 0,0);

	rend->CreateMesh(MeshLoader::FRAME);

	mat = rend->GetMaterial(1);

	mat.UVSCALE = XMFLOAT2(-1, 1);

	rend->SetMaterial(1, mat);

	isPressed = false;
}

void TitleFrameScript::OnUpdate()
{
	if (isPressed)
		return;

	if (isOnRay)
	{
		time += systemManager->GetGameDeltaTime();
	}
	else
	{
		time -= systemManager->GetGameDeltaTime();
	}

	time = std::clamp(time,0.0f, 0.1f);


	auto val = std::lerp(0.9f,1.0f , 1.0f - std::pow(time / 0.1f, 2.0f));

	mat.UVSCALE = XMFLOAT2(-val, val);
	auto adjust = 0.5f * (1.0f - val);
	mat.UVPOS = XMFLOAT2(adjust, adjust);

	rend->SetMaterial(1, mat);

	if (flag == START && !isPressed)
	{
		rend->ApplyTextureOnMesh(1, Renderer::DIFFUSE, textureLoader->LoadTexture("Textures/START.dds"));
		attachFrom.lock()->SetWorldPosition(DirectX::XMVectorSet(-1.1f, 1.2f, 0.25f, 0));
		attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(5, 5, 5, 0));
		attachFrom.lock()->SetRotation(DirectX::XMVectorSet(0, 80, 0, 0));

		if (isOnRay && KeyInput::GetInstance().GetKey(Key::MOUSE_LEFT))
		{
			auto light = systemManager->GetEntityByName("SpotLight");

			light->GetComponent<TitleSpotLightScript>()->LightUpward();

			auto ui = systemManager->GetEntityByName("TitleUI");

			ui->GetComponent<GameStartUIScript>()->OnBlack();

			isPressed = true;
		}
	}
	else if(flag == EXIT && !isPressed)
	{
		rend->ApplyTextureOnMesh(1, Renderer::DIFFUSE, textureLoader->LoadTexture("Textures/EXIT.dds"));
		attachFrom.lock()->SetWorldPosition(DirectX::XMVectorSet(1.1f, 1.2f, 0.25f, 0));
		attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(5, 5, 5, 0));
		attachFrom.lock()->SetRotation(DirectX::XMVectorSet(0, 110, 0, 0));

		if (isOnRay && KeyInput::GetInstance().GetKey(Key::MOUSE_LEFT))
		{
			systemManager->ExitGame();

			isPressed = true;
		}
	}

	isOnRay = false;
}

void TitleFrameScript::OnRayCast()
{
	isOnRay = true;
}

#include "FloorScript.h"
#include "RayCast.h"
#include "KeyInput.h"

FloorScript::FloorScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

FloorScript::~FloorScript()
{
}

void FloorScript::OnStart()
{
	attachFrom.lock()->AddComponent<SkinnedRenderer>();

	attachFrom.lock()->SetWorldPosition(DirectX::XMVectorSet(0,0,0,1));
	attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(50,1,50,1));

	sRend = attachFrom.lock()->GetComponent<SkinnedRenderer>();


	sRend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, textureLoader->LoadTexture("Textures/Floor.dds"));
	sRend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::NORMAL, textureLoader->LoadTexture("Textures/FloorN.dds"));

	mat = sRend->GetMaterial(0);

	mat.UVSCALE = XMFLOAT2(10,10);

	sRend->SetMaterial(0 , mat);

	if (sceneManager->GetCurrentGameScene()->GetName() == "CardGameScene")
	{
		mat.UVSCALE = XMFLOAT2(1, 1);

		sRend->SetMaterial(0, mat);

		attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(20, 1, 20, 1));

		sRend->CreateMesh(MeshLoader::CARPET);
	}
}

void FloorScript::OnUpdate()
{
	if (sceneManager->GetCurrentGameScene()->GetName() == "TitleScene")
	{
		mat = sRend->GetMaterial(0);

		mat.UVPOS = XMFLOAT2(0, systemManager->GetGameTime() * 0.05f);

		sRend->SetMaterial(0, mat);

		auto mousePos = KeyInput::GetInstance().GetMousePos();

		auto mousePosXm = DirectX::XMLoadFloat2(&mousePos);

		;
		auto dir = DirectX::XMVectorSubtract(Camera::GetInstance().GetPosScreenToWorld(mousePosXm), Camera::GetInstance().GetPosition());

		RayCast(Camera::GetInstance().GetPosition(), DirectX::XMVector3Normalize(dir), 100);
	}
}

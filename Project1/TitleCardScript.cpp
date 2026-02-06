#include "TitleCardScript.h"

TitleCardScript::TitleCardScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

TitleCardScript::~TitleCardScript()
{
}

void TitleCardScript::OnStart()
{
	rend = attachFrom.lock()->AddComponent<SkinnedRenderer>();

	rend->CreateMesh(MeshLoader::CARD);
	attachFrom.lock()->SetWorldPosition(DirectX::XMVectorSet(0, 1.4f, 1.350f, 0));
	attachFrom.lock()->SetWorldScale(DirectX::XMVectorSet(0.4f, 0.4f, 0.4f, 1));

	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/ace_of_spades.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/2_of_diamonds.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/3_of_hearts.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/4_of_spades.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/5_of_clubs.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/6_of_diamonds.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/7_of_hearts.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/8_of_spades.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/9_of_clubs.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/10_of_diamonds.dds"));
	texs.emplace_back(textureLoader->LoadTexture("Textures/Cards/black_joker.dds"));

	rend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, texs.front());
	rend->ApplyTextureOnMesh(1,Renderer::TEXTURE_TYPE::DIFFUSE , textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));
}

void TitleCardScript::OnUpdate()
{
	time = systemManager->GetGameTime() * 50;

	int ind = (time - 180) / 360 ;

	ind = ind % texs.size();

	rend->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, texs[ind] );

	attachFrom.lock()->SetRotation(DirectX::XMVectorSet(90, time, 0, 0));
}

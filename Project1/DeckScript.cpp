#include "DeckScript.h"
#include <random>

DeckScript::DeckScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

DeckScript::~DeckScript()
{
}

void DeckScript::Init(SIDE side)
{
	this->side = side;

	if (this->side == SIDE::PLAYER)
	{
		for (int i = 1;i < 14;i++)
		{
			for (int j = 0;j < 2;j++)
			{
				deck.emplace_back(Card((PATTERN)j , i ));
			}
		}
	}
	else
	{
		for (int i = 1;i < 14;i++)
		{
			for (int j = 2;j < 4;j++)
			{
				deck.emplace_back(Card((PATTERN)j, i));
			}
		}
	}

	ySize = deck.size();

	auto rend = attachFrom->AddComponent<SkinnedRenderer>();

	rend->CreateMesh(MeshLoader::CARD);
	rend->ApplyTextureOnMesh(1 , Renderer::DIFFUSE , textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));
	rend->ApplyTextureOnMesh(0 , Renderer::DIFFUSE , textureLoader->LoadTexture("Textures/Cards/back@2x.dds"));

	defPos = DirectX::XMVectorSet(-5, 1.3f, -2.1, 0);

	if (this->side == DeckScript::CPU)
		defPos = DirectX::XMVectorSet(5, 1.3f, 2.1, 0);

	attachFrom->SetWorldPosition(defPos);
	attachFrom->SetWorldRotation(DirectX::XMVectorSet(-180,0,0,0));
}

DeckScript::Card DeckScript::DrawCard()
{
	if (deck.size() == 0)
	{
		std::cout << "ƒfƒbƒL‚ª‚à‚¤‚È‚¢" << std::endl;
		return Card((PATTERN)5 , -1);
	}

	random_device rd; 
	mt19937 gen(rd()); 
	uniform_int_distribution<> dist(0, deck.size()-1); 

	int val = dist(gen);

	auto card = deck[val];
	deck.erase(deck.begin() + val);

	auto drawedPat = card.pat;

	switch(drawedPat)
	{
	case PATTERN::DIAMOND:
			std::cout << "DIAMOND ";
			break;
	case PATTERN::HEART:
		std::cout << "HEART ";
		break;
	case PATTERN::SPADE:
		std::cout << "SPADE ";
		break;
	case PATTERN::CLUB:
		std::cout << "CLUB ";
		break;
	}
	std::cout << card.num << std::endl;
	std::cout << deck.size() << std::endl;

	return card;
}

DirectX::XMVECTOR DeckScript::GetTopPos()
{
	return attachFrom->GetWorldPosition();
}

void DeckScript::OnStart()
{
}

void DeckScript::OnUpdate()
{
	attachFrom->SetWorldScale(DirectX::XMVectorSet(1, deck.size(), 1, 0));
}

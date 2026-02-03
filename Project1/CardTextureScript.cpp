#include "CardTextureScript.h"

CardTextureScript::CardTextureScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{

}

CardTextureScript::~CardTextureScript()
{
}


ComPtr < ID3D12Resource> CardTextureScript::GetCardTexture(DeckScript::Card card)
{
	return cardTexs[card];
}

void CardTextureScript::OnStart()
{
	std::string diamondStr = "_of_diamonds";
	std::string clubStr = "_of_clubs";
	std::string heartStr = "_of_hearts";
	std::string spadeStr = "_of_spades";

	for (int i = 0;i < 4;i++)
	{
		for (int j = 1;j < 14;j++)
		{
			ComPtr<ID3D12Resource> resource;

			std::string filePath = "Textures/Cards/";

			if (1 == j)
				filePath += "ace";
			else if (11 == j)
				filePath += "jack";
			else if (12 == j)
				filePath += "queen";
			else if (13 == j)
				filePath += "king";
			else
				filePath += to_string(j);

			if (0 == i)
				filePath += heartStr;
			else if (1 == i)
				filePath += diamondStr;
			else if (2 == i)
				filePath += spadeStr;
			else
				filePath += clubStr;

			filePath += ".dds";

			DeckScript::Card card = DeckScript::Card((DeckScript::PATTERN)i, j);

			resource = textureLoader->LoadTexture(filePath);

			cardTexs[card] = resource;
		}
	}
}

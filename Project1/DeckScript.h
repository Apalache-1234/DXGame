#ifndef DECK_SCRIPT_H
#define DECK_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"

class Entity;

class DeckScript : public ScriptComponent
{
public:
	enum PATTERN
	{
		HEART,DIAMOND,SPADE,CLUB,EMPTY
	}
	;

	enum SIDE
	{
		PLAYER,CPU
	}
	;

	struct Card
	{
		PATTERN pat;

		int num;

		Card(PATTERN pat, int num)
		{
			this->pat = pat;
			this->num = num;
		}

		Card()
		{
			this->pat = EMPTY;
			this->num = -1;
		}

		bool operator==(const Card& other) const {
			return pat == other.pat && num == other.num;
		}
	};

	DeckScript(std::shared_ptr<Entity> ent);

	~DeckScript();

	void Init(SIDE side);

	Card DrawCard();

	DirectX::XMVECTOR GetTopPos();

	void OnStart() override;

	void OnUpdate() override;
protected:
private:
	std::vector<Card> deck;

	UINT ySize;

	DirectX::XMVECTOR defPos;

	SIDE side;
};

#endif
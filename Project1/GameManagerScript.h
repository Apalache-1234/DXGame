#ifndef GAME_MANAGER_SCRIPT_H
#define GAME_MANAGER_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "SquareWipeScript.h"
#include "HandScript.h"
#include "DeckScript.h"
#include "TableDeckScript.h"
#include "Tween.h"
#include "PERadicalBlur.h"
#include "RayCast.h"

class Entity;

class GameManagerScript : public ScriptComponent
{
public:
	GameManagerScript(std::shared_ptr<Entity> ent);

	~GameManagerScript();

	void OnStart() override;

	void OnUpdate() override;
protected:
private:
	void PlaceCardsOnTableDecks();

	float startDrawTime = 0;

	const float drawInterval = 0.2f;

	UINT drawNum = 7;

	std::shared_ptr<Entity> selectCard;

	std::shared_ptr<SquareWipeScript> wipeSc;
	std::shared_ptr<HandScript>  pHandSc;
	std::shared_ptr<DeckScript>  pDeckSc;
	std::shared_ptr<HandScript>  cHandSc;
	std::shared_ptr<DeckScript>  cDeckSc;
	std::shared_ptr<TableDeckScript>  tableDeck1Sc;
	std::shared_ptr<TableDeckScript>  tableDeck2Sc;

};

#endif
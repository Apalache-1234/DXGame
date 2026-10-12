#ifndef CPU_SCRIPT_H
#define CPU_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "Tween.h"
#include "HandScript.h"
#include "DeckScript.h"

class Entity;
class TableDeckScript;
class GameManagerScript;
class PlaceCardUIScript;

class CPUScript : public ScriptComponent
{
public:
	CPUScript(std::shared_ptr<Entity> ent);

	~CPUScript();

	void OnStart() override;

	void OnUpdate() override;
protected:
private:
	std::shared_ptr<HandScript> cHandSc;
	std::shared_ptr<DeckScript> cDeckSc;

	std::shared_ptr<TableDeckScript>  tableDeck1Sc;
	std::shared_ptr<TableDeckScript>  tableDeck2Sc;

	std::shared_ptr<PlaceCardUIScript>  placeCardUISc;

	shared_ptr<GameManagerScript> gameM;


	Material mat;

	float thinkInterval1 = 2;
	float thinkInterval2 = 4;

	float collapsedTime = 0;
};

#endif
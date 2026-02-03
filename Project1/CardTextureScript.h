#ifndef CARD_TEXTURE_SCRIPT_H
#define CARD_TEXTURE_SCRIPT_H

#include <memory>
#include <typeindex>
#include "ScriptComponent.h"
#include "DeckScript.h"
#include "Tween.h"
#include "PERadicalBlur.h"

class Entity;

class CardTextureScript : public ScriptComponent
{
public:
	CardTextureScript(std::shared_ptr<Entity> ent);

	~CardTextureScript();

	void OnStart() override;

	ComPtr<ID3D12Resource> GetCardTexture(DeckScript::Card card);
protected:
private:
	struct CardHash {
		std::size_t operator()(const DeckScript::Card& k) const {
			return std::hash<int>()((int)k.pat) ^ (std::hash<int>()(k.num) << 1);
		}
	};

	std::unordered_map<DeckScript::Card, ComPtr<ID3D12Resource>, CardHash> cardTexs;
};

#endif
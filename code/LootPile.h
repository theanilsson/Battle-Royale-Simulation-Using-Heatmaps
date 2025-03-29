#pragma once
#include "Actor.h"

class LootPile : public Actor
{
public:
	LootPile() = delete;
	LootPile(const Tga::Vector2f& aPosition) : Actor(lootPileRadius, aPosition) 
	{
		mySpriteInstance.myPivot = { 0.5f, 0.5f };
		mySpriteInstance.mySize = Tga::Vector2f(lootPileRadius * 2.0f);
	};
	LootPile(const LootPile& anOtherLootPile) = delete;
	LootPile& operator=(const LootPile& anOtherLootPile) = delete;
	LootPile(LootPile&& anOtherLootPile) = default;
	LootPile& operator=(LootPile&& anOtherLootPile) = default;
	~LootPile() = default;

	void SetAsLooted();
	const bool HasBeenLooted() const;

	void Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData) override;

private:
	bool myHasBeenLooted = false;
};
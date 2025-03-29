#include "LootPile.h"
#include <tge\drawers\SpriteDrawer.h>

void LootPile::SetAsLooted()
{
	myHasBeenLooted = true;
}

const bool LootPile::HasBeenLooted() const
{
	return myHasBeenLooted;
}

void LootPile::Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData)
{
	if (!myHasBeenLooted) aSpriteDrawer.Draw(aSpriteData, mySpriteInstance);
}
#include "AirDrop.h"
#include <tge\drawers\SpriteDrawer.h>

void AirDrop::Update(const float aDeltaTime)
{
	if (!myLandingTimer.IsDone())
	{
		myLandingTimer.Update(aDeltaTime);
		if (myLandingTimer.IsDone())
		{
			myHasLanded = true;
		}
	}
}

void AirDrop::SetAsLooted()
{
	myHasBeenLooted = true;
}

const bool AirDrop::HasBeenLooted() const
{
	return myHasBeenLooted;
}

const bool AirDrop::HasLanded() const
{
	return myHasLanded;
}

void AirDrop::Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData)
{
	mySpriteInstance.myColor = myHasLanded ? Tga::Color(1.0f, 1.0f, 1.0f) : Tga::Color(1.0f, 1.0f, 1.0f, 0.3f);
	if (!myHasBeenLooted) aSpriteDrawer.Draw(aSpriteData, mySpriteInstance);
}
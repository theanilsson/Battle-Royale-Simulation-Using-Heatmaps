#include "SafeZone.h"
#include "MainSingleton.h"
#include "UtilityFunctions.h"
#include "SimulationCommon.h"
#include <tge\engine.h>
#include <tge\drawers\DebugDrawer.h>

SafeZone::~SafeZone()
{
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::ShrinkSafeZone, this);
}

void SafeZone::Init(const float aRadius, const Tga::Vector2f& aPosition)
{
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::ShrinkSafeZone, this);
	myShrinkZoneTimer.SetResetValue(4.0f);
	myCurrentRadius = aRadius;
	myOriginalPosition = aPosition;
	myCurrentPosition = aPosition;
	myRadiusStages[0] = aRadius;
	myRadiusStages[1] = aRadius * 0.75f;
	myRadiusStages[2] = aRadius * 0.6f;
	myRadiusStages[3] = aRadius * 0.45f;
	myRadiusStages[4] = aRadius * 0.30f;
	myRadiusStages[5] = aRadius * 0.15f;
	myRadiusStages[6] = aRadius * 0.075f;
	myRadiusStages[7] = aRadius * 0.0f;
}

void SafeZone::Update(const float aDeltaTime)
{
	if (!myShrinkZoneTimer.IsDone()) 
	{
		myShrinkZoneTimer.Update(aDeltaTime);
		if (myCurrentRadius > myMinimumRadius) 
		{
			myCurrentRadius -= myShrinkageScalar * aDeltaTime;
			myCurrentPosition += myShrinkDirection * myShrinkageScalar * aDeltaTime;
		} 
	}
}

void SafeZone::Receive(const Message& aMessage)
{
	switch (aMessage.messageType)
	{
		case eMessageType::ShrinkSafeZone:
		{
			myShrinkDirection = *static_cast<Tga::Vector2f*>(aMessage.messageData);
			myShrinkZoneTimer.Reset();
			if (myCurrentShrinkageStage < numberOfSafeZoneShrinkageStages)
			{
				myCurrentShrinkageStage++;
			}
			myShrinkageScalar = (myRadiusStages[myCurrentShrinkageStage - 1] - myRadiusStages[myCurrentShrinkageStage]) / myShrinkZoneTimer.GetResetValue();
			break;
		}
	}
}

void SafeZone::Reset()
{
	myCurrentShrinkageStage = 0;
	myCurrentPosition = myOriginalPosition;
	myCurrentRadius = myRadiusStages[0];
	myShrinkZoneTimer.Zeroize();
}

const float SafeZone::GetCurrentRadius() const
{
	return myCurrentRadius;
}

const Tga::Vector2f SafeZone::GetCurrentPosition() const
{
	return myCurrentPosition;
}

const bool SafeZone::IsPositionInSafeZone(const Tga::Vector2f& aPosition) const
{
	if ((aPosition - myCurrentPosition).LengthSqr() < myCurrentRadius * myCurrentRadius) return true;
	return false;
}

void SafeZone::Render()
{
	Tga::Engine::GetInstance()->GetDebugDrawer().DrawCircle(myCurrentPosition, myCurrentRadius, Tga::Color(0.75f, 0.0f, 1.0f));
}
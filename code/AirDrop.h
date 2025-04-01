#pragma once
#include "Actor.h"
#include "Timer.h"

class AirDrop : public Actor 
{
public:
	AirDrop() = delete;
	AirDrop(const Tga::Vector2f& aPosition, const Tga::Vector2f& aWindDirection, const float aWindSpeed) : Actor(airDropRadius, aPosition)
	{
		mySpriteInstance.myPivot = { 0.5f, 0.5f };
		mySpriteInstance.mySize = Tga::Vector2f(airDropRadius * 2.0f);
		mySavedWindDirection = aWindDirection;
		myWindSpeed = aWindSpeed;
		myLandingTimer.Reset();
	};
	AirDrop(const AirDrop& anOtherAirDrop) = delete;
	AirDrop& operator=(const AirDrop& anOtherAirDrop) = delete;
	AirDrop(AirDrop&& anOtherAirDrop) = default;
	AirDrop& operator=(AirDrop&& anOtherAirDrop) = default;
	~AirDrop() = default;

	void Update(const float aDeltaTime);
	void SetAsLooted();
	const bool HasBeenLooted() const;
	const bool HasLanded() const;

	void Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData) override;

private:
	CU::CountdownTimer myLandingTimer;
	Tga::Vector2f mySavedWindDirection;
	float myWindSpeed = 0.0f;
	bool myHasBeenLooted = false;
};
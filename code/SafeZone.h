#pragma once
#include <tge/math/vector2.h>
#include <array>
#include "SimulationCommon.h"
#include "Observer.h"
#include "Timer.h"

class SafeZone : public Observer
{
public:
	SafeZone() = default;
	SafeZone(const SafeZone& anOtherSafeZone) = delete;
	SafeZone& operator=(const SafeZone& anOtherSafeZone) = delete;
	SafeZone(SafeZone&& anOtherSafeZone) = default;
	SafeZone& operator=(SafeZone&& anOtherSafeZone) = default;
	~SafeZone();

	void Init(const float aRadius, const Tga::Vector2f& aPosition);

	void Update(const float aDeltaTime);
	void Receive(const Message& aMessage) override;
	void Reset();
	const float GetCurrentRadius() const;
	const Tga::Vector2f GetCurrentPosition() const;
	const bool IsPositionInSafeZone(const Tga::Vector2f& aPosition) const;

	void Render();

private:
	CU::CountdownTimer myShrinkZoneTimer;
	
	Tga::Vector2f myShrinkDirection;
	std::array<float, numberOfSafeZoneShrinkageStages + 1> myRadiusStages;
	int myCurrentShrinkageStage = 0;
	float myShrinkageScalar;
	
	Tga::Vector2f myOriginalPosition;
	Tga::Vector2f myCurrentPosition;

	float myOriginalRadius;
	float myCurrentRadius;
	const float myMinimumRadius = 1.0f;
};
#include <stdafx.h>
#include "Timer.h"

CommonUtilities::CountdownTimer::CountdownTimer()
{
	myResetValue = 1.0f;
	myCurrentValue = 0.0f;
}

CommonUtilities::CountdownTimer::CountdownTimer(const float aResetValue, const float aCurrentValue)
{
	assert(aResetValue > 0.0f && "Reset value must be greater than zero.");
	myResetValue = aResetValue;
	myCurrentValue = aCurrentValue;
	if (myCurrentValue < 0.0f) myCurrentValue = 0.0f;
}

CommonUtilities::CountdownTimer::CountdownTimer(const CountdownTimer& aTimer)
{
	myResetValue = aTimer.myResetValue;
	myCurrentValue = aTimer.myCurrentValue;
}

CommonUtilities::CountdownTimer& CommonUtilities::CountdownTimer::operator=(const CountdownTimer& aTimer)
{
	myResetValue = aTimer.myResetValue;
	myCurrentValue = aTimer.myCurrentValue;
	return *this;
}

void CommonUtilities::CountdownTimer::Update(const float aDeltaTime)
{
	if (myCurrentValue > 0.0f)
	{
		myCurrentValue = std::max(myCurrentValue - aDeltaTime, 0.0f);
	}
}

void CommonUtilities::CountdownTimer::Reset()
{
	myCurrentValue = myResetValue;
}

void CommonUtilities::CountdownTimer::Zeroize()
{
	myCurrentValue = 0.0f;
}

void CommonUtilities::CountdownTimer::SetResetValue(const float aResetValue)
{
	assert(aResetValue > 0.0f && "Reset value must be greater than zero.");
	myResetValue = aResetValue;
}

void CommonUtilities::CountdownTimer::SetCurrentValue(const float aCurrentValue)
{
	myCurrentValue = aCurrentValue;
	if (myCurrentValue < 0.0f) myCurrentValue = 0.0f;
}

const bool CommonUtilities::CountdownTimer::IsDone() const
{
	return myCurrentValue <= 0.0f;
}

const float& CommonUtilities::CountdownTimer::GetResetValue() const
{
	return myResetValue;
}

const float& CommonUtilities::CountdownTimer::GetCurrentValue() const
{
	return myCurrentValue;
}

CommonUtilities::CountupTimer::CountupTimer()
{
	myThresholdValue = 1.0f;
	myCurrentValue = 0.0f;
}

CommonUtilities::CountupTimer::CountupTimer(const float aThresholdValue, const float aCurrentValue)
{
	assert(aThresholdValue > 0.0f && "Threshold value must be greater than zero.");
	myThresholdValue = aThresholdValue;
	myCurrentValue = aCurrentValue;
	if (myCurrentValue < 0.0f) myCurrentValue = 0.0f;
}

CommonUtilities::CountupTimer::CountupTimer(const CountupTimer& aTimer)
{
	myThresholdValue = aTimer.myThresholdValue;
	myCurrentValue = aTimer.myCurrentValue;
}

CommonUtilities::CountupTimer& CommonUtilities::CountupTimer::operator=(const CountupTimer& aTimer)
{
	myThresholdValue = aTimer.myThresholdValue;
	myCurrentValue = aTimer.myCurrentValue;
	return *this;
}

void CommonUtilities::CountupTimer::Update(const float aDeltaTime)
{
	if (myCurrentValue < FLT_MAX - aDeltaTime)
	{
		myCurrentValue += aDeltaTime;
	}
}

void CommonUtilities::CountupTimer::Reset()
{
	myCurrentValue = 0.0f;
}

void CommonUtilities::CountupTimer::SetCurrentToThreshold()
{
	myCurrentValue = myThresholdValue;
}

void CommonUtilities::CountupTimer::SetThresholdValue(const float aThresholdValue)
{
	assert(aThresholdValue > 0.0f && "Threshold value must be greater than zero.");
	myThresholdValue = aThresholdValue;
}

void CommonUtilities::CountupTimer::SetCurrentValue(const float aCurrentValue)
{
	myCurrentValue = aCurrentValue;
	if (myCurrentValue < 0.0f) myCurrentValue = 0.0f;
}

const bool CommonUtilities::CountupTimer::ReachedThreshold() const
{
	return myCurrentValue >= myThresholdValue;
}

const float& CommonUtilities::CountupTimer::GetThresholdValue() const
{
	return myThresholdValue;
}

const float& CommonUtilities::CountupTimer::GetCurrentValue() const
{
	return myCurrentValue;
}
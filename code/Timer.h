#pragma once

namespace CommonUtilities
{
	class CountdownTimer
	{
	public:
		CountdownTimer();
		CountdownTimer(const float aResetValue, const float aCurrentValue = 0.0f);
		CountdownTimer(const CountdownTimer& aTimer);
		CountdownTimer& operator=(const CountdownTimer& aTimer);

		void Update(const float aDeltaTime);
		void Reset();
		void Zeroize();

		void SetResetValue(const float aResetValue);
		void SetCurrentValue(const float aCurrentValue);

		const bool IsDone() const;
		const float& GetResetValue() const;
		const float& GetCurrentValue() const;

	private:
		float myResetValue = 1.0f;
		float myCurrentValue = 0.0f;
	};

	class CountupTimer
	{
	public:
		CountupTimer();
		CountupTimer(const float aThresholdValue, const float aCurrentValue = 0.0f);
		CountupTimer(const CountupTimer& aTimer);
		CountupTimer& operator=(const CountupTimer& aTimer);

		void Update(const float aDeltaTime);
		void Reset();
		void SetCurrentToThreshold();

		void SetThresholdValue(const float aThresholdValue);
		void SetCurrentValue(const float aCurrentValue);

		const bool ReachedThreshold() const;
		const float& GetThresholdValue() const;
		const float& GetCurrentValue() const;

	private:
		float myThresholdValue = 1.0f;
		float myCurrentValue = 0.0f;
	};
}

namespace CU = CommonUtilities;
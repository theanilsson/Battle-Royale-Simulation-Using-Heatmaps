#pragma once
#include <memory>
#include "Actor.h"
#include "Timer.h"
#include "UtilityFunctions.h"

class AirDrop;
class LootPile;
class SafeZone;

class AIPawn : public Actor
{
public:
	AIPawn() = delete;
	AIPawn(const Tga::Vector2f& aPosition, const unsigned int anId) : Actor(pawnRadius, aPosition), myId(anId)
	{
		mySpriteInstance.myPivot = { 0.5f, 0.5f };
		mySpriteInstance.mySize = Tga::Vector2f(pawnRadius * 2.0f);
		RandomizeDirection();
	};
	AIPawn(const AIPawn& anOtherPawn) = delete;
	AIPawn& operator=(const AIPawn& anOtherPawn) = delete;
	AIPawn(AIPawn&& anOtherPawn) = default;
	AIPawn& operator=(AIPawn&& anOtherPawn) = default;
	~AIPawn() = default;
	void Initialize();

	void Update(const float aDeltaTime, const std::shared_ptr<SafeZone>& aSafeZone, std::vector<std::shared_ptr<LootPile>>& someLootPiles, std::vector<std::shared_ptr<AirDrop>>& someAirDrops, std::vector<std::shared_ptr<AIPawn>>& somePawns);
	void RandomizeDirection() { myMoveDirection = Tga::Vector2f(UtilityFunctions::GetRandomFloat(-10000.0f, 10000.0f), UtilityFunctions::GetRandomFloat(-10000.0f, 10000.0f)).GetNormalized(); };
	void Kill();
	const bool IsAlive() const;
	const unsigned int GetId() const;

	void Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData) override;

private:
	void EvaluateLootPileTargeting(std::vector<std::shared_ptr<LootPile>>& someLootPiles);
	void EvaluateAirDropTargeting(std::vector<std::shared_ptr<AirDrop>>& someAirDrops);
	void EvaluatePawnTargeting(std::vector<std::shared_ptr<AIPawn>>& somePawns);

	Tga::Vector2f myMoveDirection;
	CU::CountdownTimer myChangeGoalTimer;
	float myMoveSpeed = 25.0f;
	unsigned int myId;
	bool myIsAlive = true;
};
#include "AIPawn.h"
#include "AirDrop.h"
#include "LootPile.h"
#include "SafeZone.h"
#include "MainSingleton.h"
#include "UtilityFunctions.h"
#include <tge\drawers\SpriteDrawer.h>

void AIPawn::Initialize()
{
	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &mySpriteInstance.myPosition, eMessageType::PlayerSpawned });
}

void AIPawn::Update(const float aDeltaTime, const std::shared_ptr<SafeZone>& aSafeZone, std::vector<std::shared_ptr<LootPile>>& someLootPiles, std::vector<std::shared_ptr<AirDrop>>& someAirDrops, std::vector<std::shared_ptr<AIPawn>>& somePawns)
{
	myChangeGoalTimer.Update(aDeltaTime);
	if (myChangeGoalTimer.IsDone())
	{
		myChangeGoalTimer.Reset();
		myMoveDirection = (aSafeZone->GetCurrentPosition() - mySpriteInstance.myPosition).GetNormalized();

		EvaluateLootPileTargeting(someLootPiles);
		EvaluateAirDropTargeting(someAirDrops);
		EvaluatePawnTargeting(somePawns);
	}
	mySpriteInstance.myPosition += myMoveDirection * myMoveSpeed * aDeltaTime;
}

void AIPawn::Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData)
{
	if(myIsAlive) aSpriteDrawer.Draw(aSpriteData, mySpriteInstance);
}

void AIPawn::EvaluateLootPileTargeting(std::vector<std::shared_ptr<LootPile>>& someLootPiles)
{
	for (auto& lootPile : someLootPiles)
	{
		if (lootPile->HasBeenLooted()) continue;

		Tga::Vector2f lootPosition = lootPile->GetPosition();
		const Tga::Vector2f vectorToActor = lootPosition - mySpriteInstance.myPosition;
		if (vectorToActor.LengthSqr() < lootDetectionRange * lootDetectionRange)
		{
			if (vectorToActor.LengthSqr() < pickupRange * pickupRange)
			{
				lootPile->SetAsLooted();
				MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &lootPosition, eMessageType::LootPickedUp });
			}
			else
			{
				myMoveDirection = vectorToActor.GetNormalized();
			}
		}
	}
}

void AIPawn::EvaluateAirDropTargeting(std::vector<std::shared_ptr<AirDrop>>& someAirDrops)
{
	for (auto& airDrop : someAirDrops)
	{
		const Tga::Vector2f vectorToActor = airDrop->GetPosition() - mySpriteInstance.myPosition;
		if (vectorToActor.LengthSqr() < airDropDetectionRange * airDropDetectionRange)
		{
			if (airDrop->HasLanded() && vectorToActor.LengthSqr() < pickupRange * pickupRange)
			{
				airDrop->SetAsLooted();
			}
			else
			{
				myMoveDirection = vectorToActor.GetNormalized();
			}
		}
	}
}

void AIPawn::EvaluatePawnTargeting(std::vector<std::shared_ptr<AIPawn>>& somePawns)
{
	for (auto& pawn : somePawns)
	{
		if (pawn->GetPosition() == mySpriteInstance.myPosition || !pawn->IsAlive()) continue;

		const Tga::Vector2f vectorToActor = pawn->GetPosition() - mySpriteInstance.myPosition;
		if (vectorToActor.LengthSqr() < pawnAggroRange * pawnAggroRange)
		{
			if (vectorToActor.LengthSqr() < pawnKillRange * pawnKillRange)
			{
				pawn->Kill();
			}
			else
			{
				myMoveDirection = vectorToActor.GetNormalized();
			}
		}
	}
}

void AIPawn::Kill()
{
	myIsAlive = false;
	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &mySpriteInstance.myPosition, eMessageType::PlayerDied });
}

const bool AIPawn::IsAlive() const
{
	return myIsAlive;
}

const unsigned int AIPawn::GetId() const
{
	return myId;
}
#include "AIFactory.h"
#include "AIPawn.h"

std::shared_ptr<AIPawn> AIFactory::CreateSimulationPawn(const Tga::Vector2f& aPosition)
{
	return std::make_shared<AIPawn>(AIPawn(aPosition, myNextPawnId++));
}

void AIFactory::ResetNextId()
{
	myNextPawnId = 1;
}
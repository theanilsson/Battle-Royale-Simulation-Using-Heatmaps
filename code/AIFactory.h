#pragma once
#include <array>
#include <memory>
#include <tge\math\vector2.h>

class AIPawn;

class AIFactory
{
public:
	std::shared_ptr<AIPawn> CreateSimulationPawn(const Tga::Vector2f& aPosition);
	void ResetNextId();

private:
	unsigned int myNextPawnId = 1;
};
#pragma once
#include <tge\math\vector2.h>
#include <vector>
#include <array>

enum class eHeatmapType
{
	PlayerDeaths,
	PlayerSpawns,
	TimeSpentInArea,
	LootedSpots,
	AirDrops,
	Weather,
	DefaultTypeAndCount
};

enum class eWeatherType
{
	Clear,
	Cloudy,
	Rainy,
	Stormy,
	DefaultTypeAndCount
};

struct CellCluster
{
	std::vector<Tga::Vector2f> cells;
	float totalHeat = 0.0f;
};

constexpr int numPawnsToGenerate = 100;
constexpr int numLootPilesToGenerate = 50;
constexpr int numberOfSafeZoneShrinkageStages = 7;
constexpr int gridWidth = 84;
constexpr int gridHeight = 42;
constexpr float cellScale = 20.0f;

constexpr float pawnRadius = 5.0f;
constexpr float pawnAggroRange = 20.0f;
constexpr float pawnKillRange = 10.0f;
constexpr float lootDetectionRange = 40.0f;
constexpr float lootPileRadius = 10.0f;
constexpr float airDropDetectionRange = 100.0f;
constexpr float airDropRadius = 15.0f;
constexpr float pickupRange = 10.0f;
constexpr float windSpeedScalar = 75.0f;

constexpr Tga::Vector2i neighborDirections[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
constexpr std::array<float, static_cast<int>(eWeatherType::DefaultTypeAndCount)> weatherHeatValues = { 0.0f, 0.34f, 0.67f, 1.0f };
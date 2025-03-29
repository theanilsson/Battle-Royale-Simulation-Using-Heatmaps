#pragma once
#include <tge\sprite\sprite.h>
#include <tge\text\text.h>
#include "Timer.h"
#include "AIFactory.h"
#include "Observer.h"
#include "SimulationCommon.h"

class Heatmap;
class AIPawn;
class AirDrop;
class LootPile;
class SafeZone;

class Simulation : public Observer
{
public:
	Simulation() = default;
	Simulation(const Simulation& anOtherSimulation) = delete;
	Simulation& operator=(const Simulation& anOtherSimulation) = delete;
	Simulation(Simulation&& anOtherSimulation) = default;
	Simulation& operator=(Simulation&& anOtherSimulation) = default;
	~Simulation();

	void Initialize();

	bool Update(const float aDeltaTime);
	void Receive(const Message& aMessage) override;

	void Render();

private:
	void InitializeTimers();
	void InitializeInterfaceTexts(const Tga::Vector2f& aRenderSize);
	void SetHeatmapText();
	void LoadSprites(const Tga::Engine& anEngine);
	void InitializeHeatmaps();
	void SpawnPawns();
	void SpawnLoot();
	void RestartSimulation();

	void ShrinkSafeZone();
	void SpawnAirDrop();
	void UpdateSimulationEntities(const float aDeltaTime);
	void HandleSimulationFrameResults();
	void HandleInterfaceInputs();

	AIFactory myAIFactory;
	std::array<std::shared_ptr<Heatmap>, static_cast<int>(eHeatmapType::DefaultTypeAndCount)> myPreviousRoundHeatmaps;
	std::array<std::shared_ptr<Heatmap>, static_cast<int>(eHeatmapType::DefaultTypeAndCount)> myHeatmaps;
	std::array<Tga::Color, static_cast<int>(eHeatmapType::DefaultTypeAndCount)> myHeatmapColors;
	std::array<float, static_cast<int>(eHeatmapType::DefaultTypeAndCount)> myHeatmapWeights;
	std::array<float, static_cast<int>(eHeatmapType::DefaultTypeAndCount)> myHeatmapDiffusionRates;
	std::vector<std::shared_ptr<AIPawn>> myAIPawns;
	std::vector<std::shared_ptr<AirDrop>> myAirDrops;
	std::vector<std::shared_ptr<LootPile>> myLootPiles;
	std::shared_ptr<SafeZone> mySafeZone;

	Tga::SpriteSharedData myPawnSpriteData;
	Tga::SpriteSharedData myAirDropSpriteData;
	Tga::SpriteSharedData myLootPileSpriteData;
	Tga::SpriteSharedData myMapSpriteData;
	Tga::Sprite2DInstanceData myMapSpriteInstance;

	Tga::Vector2f savedDeathsPos;
	Tga::Vector2f savedTimeSpentInAreaPos;
	Tga::Vector2f savedLootedPos;
	Tga::Vector2f savedWeatherPos;
	Tga::Vector2f savedMousePos;
	Tga::Vector2f myMapMin;
	Tga::Vector2f myMapMax;

	Tga::Text myHeatmapText;
	Tga::Text mySelectionInfoText;
	Tga::Text myKeybindsText;
	Tga::Text myWinnerText;

	CU::CountdownTimer myGameRoundTimer;
	CU::CountdownTimer myShrinkSafeZoneTimer;
	CU::CountdownTimer mySpawnAirDropTimer;
	CU::CountdownTimer myUpdateWeatherTimer;

	int mySelectedHeatmap = static_cast<int>(eHeatmapType::PlayerDeaths);
	bool myIsSimViewActive = true;
};
#include "stdafx.h"
#include "Simulation.h"
#include <tge\engine.h>
#include <tge\graphics\GraphicsEngine.h>
#include <tge\texture\TextureManager.h>
#include <tge\drawers\SpriteDrawer.h>
#include "Heatmap.h"
#include "MainSingleton.h"
#include "AirDrop.h"
#include "AIPawn.h"
#include "LootPile.h"
#include "SafeZone.h"
#include "UtilityFunctions.h"

Simulation::~Simulation()
{
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerSpawned, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerDied, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::SpawnAirDrop, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::LootPickedUp, this);
}

void Simulation::Initialize()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	const Tga::Vector2f renderSize = Tga::Vector2f(static_cast<float>(engine.GetRenderSize().x), static_cast<float>(engine.GetRenderSize().y));

	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerSpawned, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerDied, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::SpawnAirDrop, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::LootPickedUp, this);

	InitializeTimers();
	InitializeInterfaceTexts(renderSize);

	LoadSprites(engine);
	myMapSpriteInstance.myPivot = { 0.5f, 0.5f };
	myMapSpriteInstance.myPosition = renderSize * 0.5f;
	myMapSpriteInstance.mySize = Tga::Vector2f(cellScale * gridWidth, cellScale * gridHeight);
	myMapSpriteInstance.myColor = Tga::Color(1.0f, 1.0f, 1.0f);
	myMapMin = myMapSpriteInstance.myPosition - myMapSpriteInstance.mySize * 0.5f;
	myMapMax = myMapSpriteInstance.myPosition + myMapSpriteInstance.mySize * 0.5f;
	
	InitializeHeatmaps();
	SpawnPawns();
	SpawnLoot();
	mySafeZone = std::make_shared<SafeZone>();
	mySafeZone->Init(1000.0f, myMapSpriteInstance.myPosition);
}

bool Simulation::Update(const float aDeltaTime)
{
	myUpdateWeatherTimer.Update(aDeltaTime);
	if (myUpdateWeatherTimer.IsDone())
	{
		myHeatmaps[static_cast<int>(eHeatmapType::Weather)]->UpdateWeather();
		myUpdateWeatherTimer.Reset();
	}

	if (!myGameRoundTimer.IsDone())
	{
		myGameRoundTimer.Update(aDeltaTime);

		myShrinkSafeZoneTimer.Update(aDeltaTime);
		if (myShrinkSafeZoneTimer.IsDone())
		{
			ShrinkSafeZone();
		}

		mySpawnAirDropTimer.Update(aDeltaTime);
		if (mySpawnAirDropTimer.IsDone())
		{
			SpawnAirDrop();
		}

		UpdateSimulationEntities(aDeltaTime);

		HandleSimulationFrameResults();
	}

	HandleInterfaceInputs();

	return MainSingleton::GetInstance()->GetInputManager().IsKeyDown(CU::eKeyCode::Escape);
}

void Simulation::Receive(const Message& aMessage)
{
	switch (aMessage.messageType)
	{
		case eMessageType::PlayerSpawned:
		{
			myHeatmaps[static_cast<int>(eHeatmapType::PlayerSpawns)]->AddHeat(*static_cast<Tga::Vector2f*>(aMessage.messageData), 1.0f);
			break;
		}
		case eMessageType::PlayerDied:
		{
			myHeatmaps[static_cast<int>(eHeatmapType::PlayerDeaths)]->AddHeat(*static_cast<Tga::Vector2f*>(aMessage.messageData), 1.0f);
			break;
		}
		case eMessageType::LootPickedUp:
		{
			myHeatmaps[static_cast<int>(eHeatmapType::LootedSpots)]->AddHeat(*static_cast<Tga::Vector2f*>(aMessage.messageData), 1.0f);
			break;
		}
	}
}

void Simulation::Render()
{
	const Tga::Engine& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer = engine.GetGraphicsEngine().GetSpriteDrawer();
	mySelectionInfoText.Render();
	myKeybindsText.Render();
	myHeatmapText.Render();
	if (myIsSimViewActive)
	{
		myMapSpriteInstance.myColor = Tga::Color(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else 
	{
		myMapSpriteInstance.myColor = Tga::Color(1.0f, 1.0f, 1.0f, 0.5f);
		myGameRoundTimer.IsDone() ? myPreviousRoundHeatmaps[static_cast<int>(mySelectedHeatmap)]->Render(spriteDrawer) : myHeatmaps[static_cast<int>(mySelectedHeatmap)]->Render(spriteDrawer);
	}
	spriteDrawer.Draw(myMapSpriteData, myMapSpriteInstance);
	for (auto& pawn : myAIPawns) 
	{
		pawn->Render(spriteDrawer, myPawnSpriteData);
	}
	for (auto& lootPile : myLootPiles) 
	{
		lootPile->Render(spriteDrawer, myLootPileSpriteData);
	}
	for (auto& airDrop : myAirDrops) 
	{
		airDrop->Render(spriteDrawer, myAirDropSpriteData);
	}
	mySafeZone->Render();

	if (myGameRoundTimer.IsDone())
	{
		myWinnerText.Render();
	}
}

void Simulation::InitializeTimers()
{
	myGameRoundTimer = CU::CountdownTimer(50.0f, 50.0f);
	myShrinkSafeZoneTimer = CU::CountdownTimer(myGameRoundTimer.GetResetValue() / (static_cast<float>(numberOfSafeZoneShrinkageStages) + 1.0f), myGameRoundTimer.GetResetValue() / (static_cast<float>(numberOfSafeZoneShrinkageStages) + 1.0f));
	mySpawnAirDropTimer = CU::CountdownTimer(myGameRoundTimer.GetResetValue() / (static_cast<float>(numberOfSafeZoneShrinkageStages) * 2.0f), myGameRoundTimer.GetResetValue() / (static_cast<float>(numberOfSafeZoneShrinkageStages) * 2.0f));
	myUpdateWeatherTimer = CU::CountdownTimer(0.075f);
}

void Simulation::InitializeInterfaceTexts(const Tga::Vector2f& aRenderSize)
{
	myWinnerText.SetPosition(Tga::Vector2f(aRenderSize.x * 0.5f - 100.0f, aRenderSize.y * 0.935f));
	myKeybindsText.SetPosition(Tga::Vector2f(aRenderSize.x * 0.065f, aRenderSize.y * 0.085f));
	myKeybindsText.SetText("Use TAB to switch between the simulation and the heatmap interface. Use Q and E to cycle which heatmap is selected. Left click to emit heat on the selected heatmap.");
	mySelectionInfoText.SetPosition(Tga::Vector2f(aRenderSize.x * 0.065f, aRenderSize.y * 0.055f));
	mySelectionInfoText.SetText("Currently selected heatmap:");
	myHeatmapText.SetPosition(Tga::Vector2f(aRenderSize.x * 0.1975f, aRenderSize.y * 0.055f));
	SetHeatmapText();
}

void Simulation::SetHeatmapText()
{
	myHeatmapText.SetColor(myHeatmapColors[mySelectedHeatmap]);
	switch (static_cast<eHeatmapType>(mySelectedHeatmap))
	{
		case eHeatmapType::PlayerDeaths:
		{
			myHeatmapText.SetText("Player Deaths");
			break;
		}
		case eHeatmapType::PlayerSpawns:
		{
			myHeatmapText.SetText("Player Spawns");
			break;
		}
		case eHeatmapType::TimeSpentInArea:
		{
			myHeatmapText.SetText("Time Spent In Area");
			break;
		}
		case eHeatmapType::LootedSpots:
		{
			myHeatmapText.SetText("Looted Spots");
			break;
		}
		case eHeatmapType::AirDrops:
		{
			myHeatmapText.SetText("Air Drops");
			break;
		}
		case eHeatmapType::Weather:
		{
			myHeatmapText.SetText("Weather");
			break;
		}
	}
}

void Simulation::LoadSprites(const Tga::Engine& anEngine)
{
	myMapSpriteData.myTexture = anEngine.GetTextureManager().GetTexture(L"Sprites/BattleRoyaleMap.png");
	myPawnSpriteData.myTexture = anEngine.GetTextureManager().GetTexture(L"Sprites/PawnSprite.dds");
	myLootPileSpriteData.myTexture = anEngine.GetTextureManager().GetTexture(L"Sprites/LootPileSprite.dds");
	myAirDropSpriteData.myTexture = anEngine.GetTextureManager().GetTexture(L"Sprites/AirDropSprite.dds");
}

void Simulation::InitializeHeatmaps()
{
	myHeatmapWeights[static_cast<int>(eHeatmapType::PlayerDeaths)] = 0.5f;
	myHeatmapWeights[static_cast<int>(eHeatmapType::PlayerSpawns)] = 1.0f;
	myHeatmapWeights[static_cast<int>(eHeatmapType::TimeSpentInArea)] = 1.0f;
	myHeatmapWeights[static_cast<int>(eHeatmapType::LootedSpots)] = 0.25f;
	myHeatmapWeights[static_cast<int>(eHeatmapType::AirDrops)] = 1.0f;
	myHeatmapWeights[static_cast<int>(eHeatmapType::Weather)] = 0.1f;

	myHeatmapDiffusionRates[static_cast<int>(eHeatmapType::PlayerDeaths)] = 0.0375f;
	myHeatmapDiffusionRates[static_cast<int>(eHeatmapType::PlayerSpawns)] = 0.015f;
	myHeatmapDiffusionRates[static_cast<int>(eHeatmapType::TimeSpentInArea)] = 0.0f;
	myHeatmapDiffusionRates[static_cast<int>(eHeatmapType::LootedSpots)] = 0.02f;
	myHeatmapDiffusionRates[static_cast<int>(eHeatmapType::AirDrops)] = 0.015f;
	myHeatmapDiffusionRates[static_cast<int>(eHeatmapType::Weather)] = 0.0f;

	myHeatmapColors[static_cast<int>(eHeatmapType::PlayerDeaths)] = Tga::Color(1.0f, 0.0f, 0.0f);
	myHeatmapColors[static_cast<int>(eHeatmapType::PlayerSpawns)] = Tga::Color(0.0f, 1.0f, 0.0f);
	myHeatmapColors[static_cast<int>(eHeatmapType::TimeSpentInArea)] = Tga::Color(0.0f, 1.0f, 0.6f);
	myHeatmapColors[static_cast<int>(eHeatmapType::LootedSpots)] = Tga::Color(1.0f, 1.0f, 0.0f);
	myHeatmapColors[static_cast<int>(eHeatmapType::AirDrops)] = Tga::Color(0.0f, 0.75f, 1.0f);
	myHeatmapColors[static_cast<int>(eHeatmapType::Weather)] = Tga::Color(0.6f, 0.6f, 0.6f);

	for (int i = 0; i < static_cast<int>(eHeatmapType::DefaultTypeAndCount); i++)
	{
		myHeatmaps[i] = std::make_shared<Heatmap>(myMapMin, cellScale, myHeatmapColors[i]);
		myPreviousRoundHeatmaps[i] = std::make_shared<Heatmap>(myMapMin, cellScale, myHeatmapColors[i]);
	}
	myHeatmaps[static_cast<int>(eHeatmapType::Weather)]->InitializeWeather();
}

void Simulation::SpawnPawns()
{
	myAIPawns.clear();
	myAIPawns.reserve(numPawnsToGenerate);
	myAIFactory.ResetNextId();

	for (int i = 0; i < numPawnsToGenerate; i++)
	{
		Tga::Vector2f position;
		bool foundColdPosToSpawnOn = false;
		while (!foundColdPosToSpawnOn)
		{
			position.x = UtilityFunctions::Lerp(myMapMin.x + pawnRadius, myMapMax.x - pawnRadius, UtilityFunctions::GetRandomFloat(0.0f, 1.0f));
			position.y = UtilityFunctions::Lerp(myMapMin.y + pawnRadius, myMapMax.y - pawnRadius, UtilityFunctions::GetRandomFloat(0.0f, 1.0f));
			foundColdPosToSpawnOn = myPreviousRoundHeatmaps[static_cast<int>(eHeatmapType::PlayerSpawns)]->IsPositionCold(position);
		}
		myAIPawns.push_back(myAIFactory.CreateSimulationPawn(position));
		myAIPawns.back()->Initialize();
	}

	// Shuffles the pawns to have no bias towards the first pawn in combats, since all pawns instakill eachother
	{
		std::random_device randomDevice;
		std::default_random_engine randomEngine(randomDevice());
		std::shuffle(std::begin(myAIPawns), std::end(myAIPawns), randomEngine);
	}
}

void Simulation::SpawnLoot()
{
	myLootPiles.clear();
	myLootPiles.reserve(numLootPilesToGenerate);

	for (int i = 0; i < numLootPilesToGenerate; i++)
	{
		Tga::Vector2f position;
		bool foundColdPosToSpawnOn = false;
		while (!foundColdPosToSpawnOn)
		{
			position.x = UtilityFunctions::Lerp(myMapMin.x + lootPileRadius, myMapMax.x - lootPileRadius, UtilityFunctions::GetRandomFloat(0.0f, 1.0f));
			position.y = UtilityFunctions::Lerp(myMapMin.y + lootPileRadius, myMapMax.y - lootPileRadius, UtilityFunctions::GetRandomFloat(0.0f, 1.0f));
			foundColdPosToSpawnOn = myPreviousRoundHeatmaps[static_cast<int>(eHeatmapType::LootedSpots)]->IsPositionCold(position);
		}
		myLootPiles.push_back(std::make_shared<LootPile>(position));
	}
}

void Simulation::RestartSimulation()
{
	myGameRoundTimer.Reset();
	myShrinkSafeZoneTimer.Reset();
	mySpawnAirDropTimer.Reset();
	mySafeZone->Reset();
	myAirDrops.clear();
	SpawnPawns();
	SpawnLoot();
}

void Simulation::ShrinkSafeZone()
{
	const Tga::Vector2f coldestSpotOnDeaths = myHeatmaps[static_cast<int>(eHeatmapType::PlayerDeaths)]->GetColdestPoint();
	const Tga::Vector2f coldestSpotOnTimeSpentInArea = myHeatmaps[static_cast<int>(eHeatmapType::TimeSpentInArea)]->GetColdestPoint();
	const Tga::Vector2f coldestSpotOnLooted = myHeatmaps[static_cast<int>(eHeatmapType::LootedSpots)]->GetColdestPoint();
	const Tga::Vector2f coldestSpotOnWeather = myHeatmaps[static_cast<int>(eHeatmapType::Weather)]->GetColdestPoint();

	const Tga::Vector2f directionToDeaths = (coldestSpotOnDeaths - mySafeZone->GetCurrentPosition()).GetNormalized() * myHeatmapWeights[static_cast<int>(eHeatmapType::PlayerDeaths)];
	const Tga::Vector2f directionToTimeSpentInArea = (coldestSpotOnTimeSpentInArea - mySafeZone->GetCurrentPosition()).GetNormalized() * myHeatmapWeights[static_cast<int>(eHeatmapType::TimeSpentInArea)];
	const Tga::Vector2f directionToLooted = (coldestSpotOnLooted - mySafeZone->GetCurrentPosition()).GetNormalized() * myHeatmapWeights[static_cast<int>(eHeatmapType::LootedSpots)];
	const Tga::Vector2f directionToWeather = (coldestSpotOnWeather - mySafeZone->GetCurrentPosition()).GetNormalized() * myHeatmapWeights[static_cast<int>(eHeatmapType::Weather)];
	
	const Tga::Vector2f resultingDirection = (directionToDeaths + directionToTimeSpentInArea + directionToLooted + directionToWeather).GetNormalized();

	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &resultingDirection , eMessageType::ShrinkSafeZone });
	myShrinkSafeZoneTimer.Reset();
}

void Simulation::SpawnAirDrop()
{
	Tga::Vector2f spawnPos;
	const Tga::Vector2f coldestFromWeather = myHeatmaps[static_cast<int>(eHeatmapType::Weather)]->FindCenterOfColdestClusterInRadius(mySafeZone->GetCurrentPosition(), mySafeZone->GetCurrentRadius() - cellScale * 2.0f);
	const Tga::Vector2f coldestFromAirDrops = myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->FindCenterOfColdestClusterInRadius(mySafeZone->GetCurrentPosition(), mySafeZone->GetCurrentRadius() - cellScale * 2.0f);
	if (coldestFromWeather == Tga::Vector2f(0.0f) || coldestFromAirDrops == Tga::Vector2f(0.0f)) spawnPos = mySafeZone->GetCurrentPosition();
	else spawnPos = (coldestFromWeather - coldestFromAirDrops) / 2.0f + coldestFromAirDrops;
	myAirDrops.push_back(std::make_shared<AirDrop>(spawnPos));
	mySpawnAirDropTimer.Reset();

	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos, 1.0f);

	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(0.0f, cellScale), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(0.0f, -cellScale), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(cellScale, 0.0f), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(-cellScale, 0.0f), 1.0f);

	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(-cellScale, cellScale), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(cellScale, -cellScale), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(cellScale, cellScale), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(-cellScale, -cellScale), 1.0f);

	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(0.0f, cellScale * 2.0f), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(0.0f, -cellScale * 2.0f), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(cellScale * 2.0f, 0.0f), 1.0f);
	myHeatmaps[static_cast<int>(eHeatmapType::AirDrops)]->AddHeat(spawnPos + Tga::Vector2f(-cellScale * 2.0f, 0.0f), 1.0f);
}

void Simulation::UpdateSimulationEntities(const float aDeltaTime)
{
	for (auto& airDrop : myAirDrops)
	{
		if (!airDrop->HasBeenLooted())
		{
			airDrop->Update(aDeltaTime);
		}
	}

	for (auto& pawn : myAIPawns)
	{
		if (pawn->IsAlive())
		{
			if (!mySafeZone->IsPositionInSafeZone(pawn->GetPosition())) pawn->Kill();

			auto previousFramePos = pawn->GetPosition();
			pawn->Update(aDeltaTime, mySafeZone, myLootPiles, myAirDrops, myAIPawns);
			auto updatedPos = pawn->GetPosition();
			if (updatedPos.x < myMapMin.x || updatedPos.x > myMapMax.x || updatedPos.y < myMapMin.y || updatedPos.y > myMapMax.y)
			{
				pawn->RandomizeDirection();
				pawn->SetPosition(previousFramePos);
			}
			myHeatmaps[static_cast<int>(eHeatmapType::TimeSpentInArea)]->AddHeat(pawn->GetPosition(), 0.2f * aDeltaTime);
		}
	}

	mySafeZone->Update(aDeltaTime);

	for (int heatmapIndex = 0; heatmapIndex < myHeatmaps.size(); heatmapIndex++)
	{
		switch (static_cast<eHeatmapType>(heatmapIndex))
		{
			case eHeatmapType::AirDrops:
			case eHeatmapType::PlayerSpawns:
			case eHeatmapType::LootedSpots:
			case eHeatmapType::PlayerDeaths:
			{
				myHeatmaps[heatmapIndex]->Update(aDeltaTime, true, myHeatmapDiffusionRates[heatmapIndex]);
				break;
			}
		}
	}
}

void Simulation::HandleSimulationFrameResults()
{
	int alivePawns = 0;
	unsigned int alivePawnId = 0;
	for (auto& pawn : myAIPawns)
	{
		if (pawn->IsAlive())
		{
			alivePawns++;
			alivePawnId = pawn->GetId();
		}
	}
	if (alivePawns == 1)
	{
		myGameRoundTimer.Zeroize();
		myWinnerText.SetText("Player #" + std::to_string(alivePawnId) + " wins!");
	}
	else if (alivePawns == 0)
	{
		myGameRoundTimer.Zeroize();
		myWinnerText.SetText("Nobody wins!");
	}

	if (myGameRoundTimer.IsDone())
	{
		for (int i = 0; i < myHeatmaps.size(); i++)
		{
			myPreviousRoundHeatmaps[i] = myHeatmaps[i];
			myHeatmaps[i] = std::make_shared<Heatmap>(myMapMin, cellScale, myHeatmapColors[i]);
		}
		myHeatmaps[static_cast<int>(eHeatmapType::Weather)]->InitializeWeather();
	}
}

void Simulation::HandleInterfaceInputs()
{
	Tga::Vector2ui windowSize = Tga::Engine::GetInstance()->GetWindowSize();
	CU::InputManager& inputManager = MainSingleton::GetInstance()->GetInputManager();
	mySavedMousePos = { static_cast<float>(inputManager.GetMousePosition().x), static_cast<float>(windowSize.y) - static_cast<float>(inputManager.GetMousePosition().y) };

	if (inputManager.IsKeyHeld(CU::eKeyCode::LeftMouseButton) || inputManager.IsKeyDown(CU::eKeyCode::LeftMouseButton))
	{
		std::shared_ptr<Heatmap>& targetHeatmap = myGameRoundTimer.IsDone() ? myPreviousRoundHeatmaps[mySelectedHeatmap] : myHeatmaps[mySelectedHeatmap];
		targetHeatmap->AddHeat(mySavedMousePos, 1.0f);
		targetHeatmap->AddHeat(mySavedMousePos + Tga::Vector2f(20.0f, 0.0f), 1.0f);
		targetHeatmap->AddHeat(mySavedMousePos + Tga::Vector2f(0.0f, 20.0f), 1.0f);
		targetHeatmap->AddHeat(mySavedMousePos + Tga::Vector2f(-20.0f, 0.0f), 1.0f);
		targetHeatmap->AddHeat(mySavedMousePos + Tga::Vector2f(0.0f, -20.0f), 1.0f);
	}

	if (inputManager.IsKeyDown(CU::eKeyCode::Space) && myGameRoundTimer.IsDone())
	{
		RestartSimulation();
	}
	else if (inputManager.IsKeyDown(CU::eKeyCode::Space))
	{
		myGameRoundTimer.Zeroize();
		for (int i = 0; i < myHeatmaps.size(); i++)
		{
			myPreviousRoundHeatmaps[i] = myHeatmaps[i];
			myHeatmaps[i] = std::make_shared<Heatmap>(myMapMin, cellScale, myHeatmapColors[i]);
		}
		myHeatmaps[static_cast<int>(eHeatmapType::Weather)]->InitializeWeather();
		myWinnerText.SetText("");
	}

	if (inputManager.IsKeyDown(CU::eKeyCode::Tab))
	{
		myIsSimViewActive = !myIsSimViewActive;
	}

	if (inputManager.IsKeyDown(CU::eKeyCode::E))
	{
		mySelectedHeatmap++;
		if (mySelectedHeatmap == static_cast<int>(eHeatmapType::DefaultTypeAndCount))
		{
			mySelectedHeatmap = 0;
		}
		SetHeatmapText();
	}
	else if (inputManager.IsKeyDown(CU::eKeyCode::Q))
	{
		mySelectedHeatmap--;
		if (mySelectedHeatmap < 0)
		{
			mySelectedHeatmap = static_cast<int>(eHeatmapType::DefaultTypeAndCount) - 1;
		}
		SetHeatmapText();
	}
}
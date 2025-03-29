#pragma once
enum class eMessageType
{
	ShrinkSafeZone,
	PlayerSpawned,
	PlayerDied,
	SpawnAirDrop,
	LootPickedUp,
	DefaultTypeAndCount
};

struct Message
{
	void* messageData = nullptr;
	eMessageType messageType = eMessageType::DefaultTypeAndCount;
};
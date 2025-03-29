#pragma once
#include <tge\input\InputManager.h>
#include "PostMaster.h"
#include "AIFactory.h"

class MainSingleton
{
public:
	static bool Start();
	static MainSingleton* GetInstance() { return myInstance; }

	CU::InputManager& GetInputManager() { return myInputManager; }
	PostMaster& GetPostMaster() { return myPostMaster; }
	AIFactory& GetEnemyFactory() { return myEnemyFactory; }

private:
	MainSingleton() {};
	static MainSingleton* myInstance;

	CU::InputManager myInputManager;
	PostMaster myPostMaster;
	AIFactory myEnemyFactory;
};
#include "MainSingleton.h"
MainSingleton* MainSingleton::myInstance = nullptr;

bool MainSingleton::Start()
{
	if (!myInstance)
	{
		myInstance = new MainSingleton();
		return true;
	}
	else
	{
		return false;
	}
}
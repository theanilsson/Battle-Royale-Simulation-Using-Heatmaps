#include "PostMaster.h"
#include "Observer.h"

void PostMaster::Subscribe(const eMessageType aMessageType, Observer* anObserver)
{
	myObservers[static_cast<int>(aMessageType)].push_back(anObserver);
}

void PostMaster::Unsubscribe(const eMessageType aMessageType, Observer* anObserver)
{
	for(size_t i = 0; i < myObservers[static_cast<int>(aMessageType)].size(); i++)
	{
		if(myObservers[static_cast<int>(aMessageType)][i] == anObserver)
		{
			myObservers[static_cast<int>(aMessageType)].erase(myObservers[static_cast<int>(aMessageType)].begin() + i);
			break;
		}
	}
}

void PostMaster::TriggerMessage(Message aMessage)
{
	for(int i = 0; i < myObservers[static_cast<int>(aMessage.messageType)].size(); ++i)
	{
		if(myObservers[static_cast<int>(aMessage.messageType)][i] != nullptr)
		{
			myObservers[static_cast<int>(aMessage.messageType)][i]->Receive(aMessage);
		}
	}
}
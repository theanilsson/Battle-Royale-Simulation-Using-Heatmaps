#pragma once
struct Message;

class Observer
{
public:
	virtual void Receive(const Message& aMessage) = 0;
};
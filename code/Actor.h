#pragma once
#include <tge\graphics\GraphicsEngine.h>
#include <tge\sprite\sprite.h>
#include "SimulationCommon.h"

namespace Tga 
{
	class SpriteDrawer;
}

class Actor 
{
public:
	Actor() = delete;
	Actor(const float aRadius, const Tga::Vector2f& aPosition) : myRadius(aRadius) { mySpriteInstance.myPosition = aPosition; };
	Actor(const Actor& anOtherActor) = delete;
	Actor& operator=(const Actor& anOtherActor) = delete;
	Actor(Actor&& anOtherActor) = default;
	Actor& operator=(Actor&& anOtherActor) = default;
	~Actor() = default;

	const Tga::Vector2f GetPosition() const { return mySpriteInstance.myPosition; };
	void SetPosition(const Tga::Vector2f& aPosition) { mySpriteInstance.myPosition = aPosition; };

	virtual void Render(Tga::SpriteDrawer& aSpriteDrawer, const Tga::SpriteSharedData& aSpriteData) = 0;

protected:
	Tga::Sprite2DInstanceData mySpriteInstance;
	float myRadius;
};
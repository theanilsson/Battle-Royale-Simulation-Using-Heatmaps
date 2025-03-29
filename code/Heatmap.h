#pragma once
#include <tge\sprite\sprite.h>
#include "SimulationCommon.h"

namespace Tga
{
	class SpriteDrawer;
}
class AIPawn;

class Heatmap 
{
public:
	Heatmap() = delete;
	Heatmap(const Tga::Vector2f& aBottomLeftPoint, const float aCellScale, const Tga::Color& aColor);
	Heatmap(const Heatmap& anOtherHeatmap) = delete;
	Heatmap& operator=(const Heatmap& anOtherHeatmap) = delete;
	Heatmap(Heatmap&& anOtherHeatmap) = default;
	Heatmap& operator=(Heatmap&& anOtherHeatmap) = default;
	~Heatmap() = default;

	void InitializeWeather();
	void Update(const float aDeltaTime, const bool aShouldDiffuse, const float aDecayOrDiffusionRate);
	void UpdateWeather();
	void AddHeat(const Tga::Vector2f& aPosition, const float anAmount);
	void ClearHeat();
	const bool IsPositionCold(const Tga::Vector2f& aPosition) const;
	const Tga::Vector2f GetColdestPoint();
	const Tga::Vector2f GetHottestPoint();
	Tga::Vector2f FindCenterOfColdestClusterInRadius(const Tga::Vector2f& aPosition, const float aRadius);

	void Render(Tga::SpriteDrawer& aSpriteDrawer);

private:
	Tga::Vector2f FindBestClusterCenterByHeat(const bool aShouldFindHottest, const float aThreshold);
	std::vector<CellCluster> SplitCluster(const CellCluster& aCluster, const int aNumberOfSplits);

	struct Cell
	{
		Tga::Vector2f minPoint;
		Tga::Vector2f maxPoint;
		Tga::Sprite2DInstanceData spriteInstance;
		float heat = 0.0f;
	};
	std::array<std::array<Cell, gridHeight>, gridWidth> myCells;
	Tga::SpriteSharedData myCellSprite;
	Tga::Color myMainColor;
};
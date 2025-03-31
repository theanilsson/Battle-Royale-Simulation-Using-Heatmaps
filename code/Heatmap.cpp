#include "Heatmap.h"
#include "AIPawn.h"
#include "UtilityFunctions.h"
#include <tge\engine.h>
#include <tge\graphics\GraphicsEngine.h>
#include <tge\drawers\LineDrawer.h>
#include <tge\drawers\SpriteDrawer.h>
#include <tge\primitives\LinePrimitive.h>
#include <tge\texture\TextureManager.h>
#include <queue>
#include <algorithm>
#include <random>

Heatmap::Heatmap(const Tga::Vector2f& aBottomLeftPoint, const float aCellScale, const Tga::Color& aColor) : myMainColor(aColor)
{
	myCellSprite.myTexture = Tga::Engine::GetInstance()->GetTextureManager().GetTexture(L"Sprites/CellSprite.dds");
	for (int x = 0; x < myCells.size(); x++)
	{
		for (int y = 0; y < myCells[x].size(); y++)
		{
			myCells[x][y].minPoint = Tga::Vector2f(aBottomLeftPoint.x + aCellScale * x, aBottomLeftPoint.y + aCellScale * y);
			myCells[x][y].maxPoint = Tga::Vector2f(aBottomLeftPoint.x + aCellScale + aCellScale * x, aBottomLeftPoint.y + aCellScale + aCellScale * y);
			myCells[x][y].spriteInstance.myPosition.x = myCells[x][y].minPoint.x + aCellScale / 2.0f;
			myCells[x][y].spriteInstance.myPosition.y = myCells[x][y].minPoint.y + aCellScale / 2.0f;
			myCells[x][y].spriteInstance.myPivot = { 0.5f, 0.5f };
			myCells[x][y].spriteInstance.mySize = Tga::Vector2f(aCellScale);
			myCells[x][y].spriteInstance.myColor = aColor;
		}
	}
}

void Heatmap::InitializeWeather()
{
	for (int x = 0; x < gridWidth; x++)
	{
		for (int y = 0; y < gridHeight; y++) 
		{
			float randomValue = UtilityFunctions::GetRandomFloat(0.0f, 1.0f);
			if (randomValue < 0.7f) myCells[x][y].heat = weatherHeatValues[static_cast<int>(eWeatherType::Clear)];
			else if (randomValue < 0.85f) myCells[x][y].heat = weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)];
			else if (randomValue < 0.95f) myCells[x][y].heat = weatherHeatValues[static_cast<int>(eWeatherType::Rainy)];
			else myCells[x][y].heat = weatherHeatValues[static_cast<int>(eWeatherType::Stormy)];
		}
	}
}

void Heatmap::Update(const float aDeltaTime, const bool aShouldDiffuse, const float aDecayOrDiffusionRate)
{
	for (int x = 0; x < gridWidth; x++)
	{
		for (int y = 0; y < gridHeight; y++)
		{
			if (aShouldDiffuse)
			{
				if (x > 0) myCells[x][y].heat += aDecayOrDiffusionRate * myCells[x - 1][y].heat * aDeltaTime;
				if (x < gridWidth - 1) myCells[x][y].heat += aDecayOrDiffusionRate * myCells[x + 1][y].heat * aDeltaTime;
				if (y > 0) myCells[x][y].heat += aDecayOrDiffusionRate * myCells[x][y - 1].heat * aDeltaTime;
				if (y < gridHeight - 1) myCells[x][y].heat += aDecayOrDiffusionRate * myCells[x][y + 1].heat * aDeltaTime;
			}
			else if(myCells[x][y].heat > 0.0f)
			{
				myCells[x][y].heat -= aDecayOrDiffusionRate * aDeltaTime;
				if (myCells[x][y].heat < 0.0f)
				{
					myCells[x][y].heat = 0.0f;
				}
			}
		}
	}
}

void Heatmap::UpdateWeather()
{
	std::array<std::array<Cell, gridHeight>, gridWidth>* copiedGrid = new std::array<std::array<Cell, gridHeight>, gridWidth>(myCells);

	for (int x = 0; x < gridWidth; x++) 
	{
		for (int y = 0; y < gridHeight; y++) 
		{
			int stormyNeighbors = 0;
			int rainyNeighbors = 0;
			int cloudyNeighbors = 0;
			for (int directionX = -1; directionX <= 1; directionX++) 
			{
				for (int directionY = -1; directionY <= 1; directionY++) 
				{
					if (directionX == 0 && directionY == 0) continue;
					const int neighborX = x + directionX;
					const int neighborY = y + directionY;
					if (neighborX >= 0 && neighborX < gridWidth && neighborY >= 0 && neighborY < gridHeight) 
					{
						if (myCells[neighborX][neighborY].heat == weatherHeatValues[static_cast<int>(eWeatherType::Stormy)]) stormyNeighbors++;
						else if (myCells[neighborX][neighborY].heat == weatherHeatValues[static_cast<int>(eWeatherType::Rainy)]) rainyNeighbors++;
						else if (myCells[neighborX][neighborY].heat == weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)]) cloudyNeighbors++;
					}
				}
			}

			float updatedHeat = myCells[x][y].heat;
			if (myCells[x][y].heat == weatherHeatValues[static_cast<int>(eWeatherType::Clear)] && cloudyNeighbors > 2)
				updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)];
			if (myCells[x][y].heat == weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)] && rainyNeighbors > 1)
				updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Rainy)];
			if (myCells[x][y].heat == weatherHeatValues[static_cast<int>(eWeatherType::Rainy)] && stormyNeighbors > 2)
				updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Stormy)];

			if (myCells[x][y].heat == weatherHeatValues[static_cast<int>(eWeatherType::Stormy)] && stormyNeighbors < 1)
				updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Rainy)];
			if (myCells[x][y].heat == weatherHeatValues[static_cast<int>(eWeatherType::Rainy)] && rainyNeighbors < 2)
				updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)];
			if (myCells[x][y].heat == weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)] && cloudyNeighbors < 1)
				updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Clear)];

			float randomDecay = UtilityFunctions::GetRandomFloat(0.0f, 1.0f);
			if (randomDecay < 0.01f && updatedHeat > weatherHeatValues[static_cast<int>(eWeatherType::Clear)]) 
			{
				if (updatedHeat == weatherHeatValues[static_cast<int>(eWeatherType::Stormy)]) updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Rainy)];
				else if (updatedHeat == weatherHeatValues[static_cast<int>(eWeatherType::Rainy)]) updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)];
				else if (updatedHeat == weatherHeatValues[static_cast<int>(eWeatherType::Cloudy)]) updatedHeat = weatherHeatValues[static_cast<int>(eWeatherType::Clear)];
			}

			(*copiedGrid)[x][y].heat = updatedHeat;
		}
	}

	myCells = *copiedGrid;
	delete copiedGrid;
}

void Heatmap::AddHeat(const Tga::Vector2f& aPosition, const float anAmount)
{
	const int cellX = static_cast<int>((aPosition.x - myCells[0][0].minPoint.x) / cellScale);
	const int cellY = static_cast<int>((aPosition.y - myCells[0][0].minPoint.y) / cellScale);

	if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight)
	{
		myCells[cellX][cellY].heat = UtilityFunctions::Min(1.0f, myCells[cellX][cellY].heat + anAmount);
	}
}

void Heatmap::ClearHeat()
{
	for (int x = 0; x < myCells.size(); x++)
	{
		for (int y = 0; y < myCells[x].size(); y++)
		{
			myCells[x][y].heat = 0.0f;
		}
	}
}

const bool Heatmap::IsPositionCold(const Tga::Vector2f& aPosition) const
{
	const int cellX = static_cast<int>((aPosition.x - myCells[0][0].minPoint.x) / cellScale);
	const int cellY = static_cast<int>((aPosition.y - myCells[0][0].minPoint.y) / cellScale);

	if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight && myCells[cellX][cellY].heat < 0.05f)
	{
		return true;
	}
	return false;
}

const Tga::Vector2f Heatmap::GetColdestPoint()
{
	return FindBestClusterCenterByHeat(false, 0.01f);
}

const Tga::Vector2f Heatmap::GetHottestPoint()
{
	return FindBestClusterCenterByHeat(true, 0.8f);
}

Tga::Vector2f Heatmap::FindBestClusterCenterByHeat(const bool aShouldFindHottest, const float aThreshold)
{
	std::vector<std::vector<bool>> visited(gridWidth, std::vector<bool>(gridHeight, false));
	std::vector<CellCluster> clusters;

	for (int iterationCellX = 0; iterationCellX < gridWidth; iterationCellX++)
	{
		for (int iterationCellY = 0; iterationCellY < gridHeight; iterationCellY++)
		{
			if (visited[iterationCellX][iterationCellY]) continue;

			float heat = myCells[iterationCellX][iterationCellY].heat;
			if ((aShouldFindHottest && heat < aThreshold) || (!aShouldFindHottest && heat > aThreshold)) continue;

			CellCluster cluster;
			std::queue<std::pair<int, int>> queue;
			queue.push({ iterationCellX, iterationCellY });
			visited[iterationCellX][iterationCellY] = true;

			while (!queue.empty())
			{
				const int queueCellX = queue.front().first;
				const int queueCellY = queue.front().second;
				queue.pop();

				cluster.cells.push_back(myCells[queueCellX][queueCellY].spriteInstance.myPosition);
				cluster.totalHeat += myCells[queueCellX][queueCellY].heat;

				for (int directionIndex = 0; directionIndex < 4; directionIndex++)
				{
					const int neighborCellX = queueCellX + neighborDirections[directionIndex].x;
					const int neighborCellY = queueCellY + neighborDirections[directionIndex].y;

					if (neighborCellX >= 0 && neighborCellX < gridWidth && neighborCellY >= 0 && neighborCellY < gridHeight && !visited[neighborCellX][neighborCellY])
					{
						float neighborHeat = myCells[neighborCellX][neighborCellY].heat;
						if ((aShouldFindHottest && neighborHeat >= aThreshold) || (!aShouldFindHottest && neighborHeat <= aThreshold))
						{
							queue.push({ neighborCellX, neighborCellY });
							visited[neighborCellX][neighborCellY] = true;
						}
					}
				}
			}

			clusters.push_back(cluster);
		}
	}

	for (size_t i = 0; i < clusters.size(); i++)
	{
		if (clusters[i].cells.size() > 200)
		{
			auto splitClusters = SplitCluster(clusters[i], static_cast<int>(clusters[i].cells.size()) / 200 + 1);
			clusters[i] = clusters.back();
			clusters.pop_back();
			for (auto& cluster : splitClusters)
			{
				clusters.push_back(cluster);
			}
		}
	}

	// Shuffles the clusters to have no bias towards the bottom left of the grid if there are multiple clusters with the same total heat value and amount of cells
	{
		std::random_device randomDevice;
		std::default_random_engine randomEngine(randomDevice());
		std::shuffle(std::begin(clusters), std::end(clusters), randomEngine);
	}

	float bestHeat = -1.0f;
	for (auto& cluster : clusters)
	{
		if (bestHeat == -1.0f || ((!aShouldFindHottest && cluster.totalHeat < bestHeat) || (aShouldFindHottest && cluster.totalHeat > bestHeat)))
		{
			bestHeat = cluster.totalHeat;
		}
	}

	std::vector<CellCluster*> bestClusters;
	for (auto& cluster : clusters)
	{
		if (cluster.totalHeat == bestHeat)
		{
			bestClusters.push_back(&cluster);
		}
	}

	CellCluster* biggestCluster = nullptr;
	for (auto& cluster : bestClusters)
	{
		if (biggestCluster == nullptr || cluster->cells.size() > biggestCluster->cells.size())
		{
			biggestCluster = cluster;
		}
	}

	if (biggestCluster != nullptr && !biggestCluster->cells.empty())
	{
		Tga::Vector2f clusterCenter(0.0f, 0.0f);
		for (auto& pos : biggestCluster->cells)
		{
			clusterCenter.x += pos.x;
			clusterCenter.y += pos.y;
		}
		clusterCenter.x /= biggestCluster->cells.size();
		clusterCenter.y /= biggestCluster->cells.size();
		return clusterCenter;
	}

	return Tga::Vector2f(0.0f, 0.0f);
}

Tga::Vector2f Heatmap::FindCenterOfColdestClusterInRadius(const Tga::Vector2f& aPosition, const float aRadius)
{
	std::vector<std::vector<bool>> visited(gridWidth, std::vector<bool>(gridHeight, false));
	std::vector<CellCluster> clusters;
	const int centerX = static_cast<int>((aPosition.x - myCells[0][0].minPoint.x) / cellScale);
	const int centerY = static_cast<int>((aPosition.y - myCells[0][0].minPoint.y) / cellScale);
	const int cellRadius = static_cast<int>(aRadius / cellScale);
	const float radiusSquared = aRadius * aRadius;

	for (int iterationCellX = UtilityFunctions::Max(0, centerX - cellRadius); iterationCellX <= UtilityFunctions::Min(gridWidth - 1, centerX + cellRadius); iterationCellX++)
	{
		for (int iterationCellY = UtilityFunctions::Max(0, centerY - cellRadius); iterationCellY <= UtilityFunctions::Min(gridHeight - 1, centerY + cellRadius); iterationCellY++)
		{
			if (myCells[iterationCellX][iterationCellY].heat > 0.01f) continue;
			
			if (visited[iterationCellX][iterationCellY]) continue;

			Tga::Vector2f vectorToIterationCell = myCells[iterationCellX][iterationCellY].spriteInstance.myPosition - aPosition;
			if ((vectorToIterationCell.x * vectorToIterationCell.x + vectorToIterationCell.y * vectorToIterationCell.y) > radiusSquared) continue;

			CellCluster cluster;
			std::queue<std::pair<int, int>> queue;
			queue.push({ iterationCellX, iterationCellY });
			visited[iterationCellX][iterationCellY] = true;

			while (!queue.empty())
			{
				const int queueCellX = queue.front().first;
				const int queueCellY = queue.front().second;
				queue.pop();

				Tga::Vector2f vecToQueueCell = myCells[queueCellX][queueCellY].spriteInstance.myPosition - aPosition;
				if ((vecToQueueCell.x * vecToQueueCell.x + vecToQueueCell.y * vecToQueueCell.y) > radiusSquared) continue;

				cluster.cells.push_back(myCells[queueCellX][queueCellY].spriteInstance.myPosition);
				cluster.totalHeat += myCells[queueCellX][queueCellY].heat;

				for (int directionIndex = 0; directionIndex < 4; directionIndex++)
				{
					const int neighborX = queueCellX + neighborDirections[directionIndex].x;
					const int neighborY = queueCellY + neighborDirections[directionIndex].y;

					if (neighborX >= 0 && neighborX < gridWidth && neighborY >= 0 && neighborY < gridHeight && !visited[neighborX][neighborY])
					{
						float neighborHeat = myCells[neighborX][neighborY].heat;
						Tga::Vector2f vecToNeigborCell = myCells[neighborX][neighborY].spriteInstance.myPosition - aPosition;

						if (neighborHeat <= 0.01f && (vecToNeigborCell.x * vecToNeigborCell.x + vecToNeigborCell.y * vecToNeigborCell.y <= radiusSquared))
						{
							queue.push({ neighborX, neighborY });
							visited[neighborX][neighborY] = true;
						}
					}
				}
			}

			clusters.push_back(cluster);
		}
	}

	for (size_t i = 0; i < clusters.size(); i++)
	{
		if (clusters[i].cells.size() > 10)
		{
			auto splitClusters = SplitCluster(clusters[i], static_cast<int>(clusters[i].cells.size()) / 10 + 1);
			clusters[i] = clusters.back();
			clusters.pop_back();
			for (auto& cluster : splitClusters)
			{
				clusters.push_back(cluster);
			}
		}
	}

	// Shuffles the clusters to have no bias towards the bottom left of the grid if there are multiple clusters with the same total heat value
	{
		std::random_device randomDevice;
		std::default_random_engine randomEngine(randomDevice());
		std::shuffle(std::begin(clusters), std::end(clusters), randomEngine);
	}


	CellCluster* coldestCluster = nullptr;
	for (auto& cluster : clusters)
	{
		if (!coldestCluster || cluster.totalHeat < coldestCluster->totalHeat)
		{
			coldestCluster = &cluster;
		}
	}

	if (coldestCluster && !coldestCluster->cells.empty())
	{
		Tga::Vector2f clusterCenter(0.0f, 0.0f);
		for (auto& pos : coldestCluster->cells)
		{
			clusterCenter.x += pos.x;
			clusterCenter.y += pos.y;
		}
		clusterCenter.x /= coldestCluster->cells.size();
		clusterCenter.y /= coldestCluster->cells.size();
		return clusterCenter;
	}

	return Tga::Vector2f(0.0f, 0.0f);
}

std::vector<CellCluster> Heatmap::SplitCluster(const CellCluster& aCluster, const int aNumberOfSplits)
{
	std::vector<CellCluster> splitClusters;
	const int step = static_cast<int>(aCluster.cells.size()) / aNumberOfSplits;

	for (int i = 0; i < aNumberOfSplits; ++i)
	{
		CellCluster subCluster;
		for (int j = i * step; j < (i + 1) * step && j < aCluster.cells.size(); ++j)
		{
			subCluster.cells.push_back(aCluster.cells[j]);
			const int cellX = static_cast<int>((aCluster.cells[j].x - myCells[0][0].minPoint.x) / cellScale);
			const int cellY = static_cast<int>((aCluster.cells[j].y - myCells[0][0].minPoint.y) / cellScale);

			if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight)
			{
				subCluster.totalHeat += myCells[cellX][cellY].heat;
			}
		}
		splitClusters.push_back(subCluster);
	}

	return splitClusters;
}

void Heatmap::Render(Tga::SpriteDrawer& aSpriteDrawer)
{
	std::vector<Tga::Sprite2DInstanceData> instances;
	instances.reserve(4 * gridWidth * gridHeight);
	for (int x = 0; x < myCells.size(); x++)
	{
		for (int y = 0; y < myCells[x].size(); y++)
		{
			myCells[x][y].spriteInstance.myColor = Tga::Color(myMainColor.myR, myMainColor.myG, myMainColor.myB, myCells[x][y].heat);
			instances.push_back(myCells[x][y].spriteInstance);
		}
	}
	aSpriteDrawer.Draw(myCellSprite, &instances[0], instances.size());
}

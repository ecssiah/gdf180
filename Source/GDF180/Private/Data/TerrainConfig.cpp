#include "TerrainConfig.h"

UTerrainConfig::UTerrainConfig()
	:
	Seed { 813 },
	CellSizeInCentimeters { 200.0f },
	SectorSizeInCells { 20 },
	WorldSizeInSectors { 8 },
	WaterLevel { 0.0f },
	NoiseGroupArray {
		{
			TEXT("Terrain"),
			{
				{
					1.0f,
					5000.0f,
					2000.0f,
				},
				{
					1.0f,
					200.0f,
					20.0f,
				},
			},
		},
		{
			TEXT("Water"),
			{
				{
					1.0f,
					100.0f,
					20.0f,
				},
			}
		},
	}
{}

float UTerrainConfig::GetSectorSizeInCentimeters() const
{
	return CellSizeInCentimeters * SectorSizeInCells;
}

uint32 UTerrainConfig::GetSectorCellNum() const
{
	return SectorSizeInCells * SectorSizeInCells;
}
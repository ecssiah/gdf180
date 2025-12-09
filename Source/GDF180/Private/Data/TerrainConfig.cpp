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
	},
	bDebugBiomes { false },
	BiomePeriod { 10000.0f },
	BiomeDefinitionArray { 
		{
			{
				TEXT("b0_grass"),
				{ 0.0f, 1.0f, 0.0f, 1.0f },
			},
			{
				TEXT("b1_moss"),
				{ 1.0f, 1.0f, 0.0f, 1.0f },
			},
			{
				TEXT("b2_lichen"),
				{ 0.2f, 0.2f, 0.2f, 1.0f },
			},
			{
				TEXT("b3_redsands"),
				{ 0.0f, 1.0f, 0.0f, 1.0f },
			},
			{
				TEXT("b4_mountain"),
				{ 1.0f, 1.0f, 0.0f, 1.0f },
			},
			{
				TEXT("b5_snow"),
				{ 0.2f, 0.2f, 0.2f, 1.0f },
			},
		} 
	}
{ }

float UTerrainConfig::GetSectorSizeInCentimeters() const
{
	return CellSizeInCentimeters * SectorSizeInCells;
}
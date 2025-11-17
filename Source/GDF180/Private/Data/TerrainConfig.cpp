#include "TerrainConfig.h"

UTerrainConfig::UTerrainConfig()
	:
	Seed { 813 },
	CellSizeInCentimeters { 200.0f },
	SectorSizeInCells { 20 },
	WorldSizeInSectors { 8 },
	WaterLevel { 0.0f },
	NoiseGroupArray { TArray<FNoiseGroup>() },
	bDebugBiomes { false },
	BiomePeriod { 10000.0f },
	BiomeDefinitionArray { TArray<FBiomeDefinition>() }
{
	FNoiseGroup TerrainNoiseGroup;
	TerrainNoiseGroup.Name = TEXT("Terrain");
	TerrainNoiseGroup.NoiseLayerArray = TArray<FNoiseLayer>();
	
	FNoiseLayer TerrainNoiseLayer0;
	TerrainNoiseLayer0.Weight = 1.0f;
	TerrainNoiseLayer0.Period = 5000.0f;
	TerrainNoiseLayer0.Amplitude = 1000.0f;
		
	TerrainNoiseGroup.NoiseLayerArray.Add(TerrainNoiseLayer0);
		
	FNoiseGroup WaterNoiseGroup;
	WaterNoiseGroup.Name = TEXT("Water");
	WaterNoiseGroup.NoiseLayerArray = TArray<FNoiseLayer>();
	
	FNoiseLayer WaterNoiseLayer0;
	WaterNoiseLayer0.Weight = 1.0f;
	WaterNoiseLayer0.Period = 100.0f;
	WaterNoiseLayer0.Amplitude = 20.0f;
	
	WaterNoiseGroup.NoiseLayerArray.Add(WaterNoiseLayer0);

	NoiseGroupArray.Add(TerrainNoiseGroup);
	NoiseGroupArray.Add(WaterNoiseGroup);
	
	FBiomeDefinition GrassBiomeDefinition;
	GrassBiomeDefinition.Name = TEXT("Grass");
	GrassBiomeDefinition.DebugColor = { 0.0f, 1.0f, 0.0f, 1.0f };

	FBiomeDefinition SandBiomeDefinition;
	SandBiomeDefinition.Name = TEXT("Sand");
	SandBiomeDefinition.DebugColor = { 1.0f, 1.0f, 0.0f, 1.0f };

	FBiomeDefinition RockBiomeDefinition;
	RockBiomeDefinition.Name = TEXT("Rock");
	RockBiomeDefinition.DebugColor = { 0.2f, 0.2f, 0.2f, 1.0f };

	BiomeDefinitionArray.Add(GrassBiomeDefinition);
	BiomeDefinitionArray.Add(SandBiomeDefinition);
	BiomeDefinitionArray.Add(RockBiomeDefinition);
}

float UTerrainConfig::GetSectorSizeInCentimeters() const
{
	return CellSizeInCentimeters * SectorSizeInCells;
}
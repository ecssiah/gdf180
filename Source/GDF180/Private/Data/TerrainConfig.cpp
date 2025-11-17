#include "TerrainConfig.h"

UTerrainConfig::UTerrainConfig()
	:
	Seed { 813 },
	CellSizeInCentimeters { 100.0f },
	SectorSizeInCells { 20 },
	WorldSizeInSectors { 12 },
	WaterLevel { 0.0f },
	NoiseGroupArray { TArray<FNoiseGroup>() },
	bDebugBiomes { false },
	BiomePeriod { 100.0f },
	BiomeDefinitionArray { TArray<FBiomeDefinition>() }
{
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
		
	FNoiseGroup TerrainNoiseGroup;
	TerrainNoiseGroup.Name = TEXT("Terrain");
	TerrainNoiseGroup.NoiseLayerArray = TArray<FNoiseLayer>();
	
	FNoiseLayer TerrainNoiseLayer0;
	TerrainNoiseLayer0.Weight = 1.0f;
	TerrainNoiseLayer0.Period = 200.0f;
	TerrainNoiseLayer0.Amplitude = 2000.0f;
		
	FNoiseLayer TerrainNoiseLayer1;
	TerrainNoiseLayer1.Weight = 1.0f;
	TerrainNoiseLayer1.Period = 100.0f;
	TerrainNoiseLayer1.Amplitude = 100.0f;
	
	TerrainNoiseGroup.NoiseLayerArray.Add(TerrainNoiseLayer0);
	TerrainNoiseGroup.NoiseLayerArray.Add(TerrainNoiseLayer1);
		
	FNoiseGroup WaterNoiseGroup;
	WaterNoiseGroup.Name = TEXT("Water");
	WaterNoiseGroup.NoiseLayerArray = TArray<FNoiseLayer>();
	
	FNoiseLayer WaterNoiseLayer0;
	WaterNoiseLayer0.Weight = 1.0f;
	WaterNoiseLayer0.Period = 100.0f;
	WaterNoiseLayer0.Amplitude = 200.0f;
	
	WaterNoiseGroup.NoiseLayerArray.Add(WaterNoiseLayer0);

	NoiseGroupArray.Add(TerrainNoiseGroup);
	NoiseGroupArray.Add(WaterNoiseGroup);
}

float UTerrainConfig::GetSectorSizeInCentimeters() const
{
	return CellSizeInCentimeters * SectorSizeInCells;
}
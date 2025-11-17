#pragma once

#include "CoreMinimal.h"
#include "BiomeDefinition.h"
#include "NoiseGroup.h"
#include "TerrainConfig.generated.h"


UCLASS()
class UTerrainConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UTerrainConfig();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Seed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CellSizeInCentimeters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SectorSizeInCells;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 WorldSizeInSectors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float WaterLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNoiseGroup> NoiseGroupArray;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDebugBiomes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BiomePeriod;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FBiomeDefinition> BiomeDefinitionArray;

	float GetSectorSizeInCentimeters() const;
};
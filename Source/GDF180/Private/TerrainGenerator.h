#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkyLightComponent.h"
#include "../ThirdParty/FastNoiseLite/FastNoiseLite.h"
#include "Components/SectorComponent.h"
#include "Data/SectorMeshes.h"
#include "Data/SectorRenderData.h"
#include "Data/TerrainConfig.h"
#include "TerrainGenerator.generated.h"


UCLASS()
class ATerrainGenerator : public AActor
{
	GENERATED_BODY()

public:	
	ATerrainGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<UTerrainConfig> TerrainConfig;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	TObjectPtr<UMaterialInterface> GroundMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	TObjectPtr<UMaterialInterface> WaterMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	TObjectPtr<UMaterialInterface> BiomesMaterial;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkyLightComponent> SkyLightComponent;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	FTimerHandle StreamingTimer;
	
	FastNoiseLite TerrainNoise;
	FastNoiseLite BiomeNoise;

	const FNoiseGroup* TerrainNoiseGroup;
	const FNoiseGroup* WaterNoiseGroup;

	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<USectorComponent>> ActiveSectorMap;
	
	TMap<FIntPoint, FSectorRenderData> SectorRenderDataMap;

	UPROPERTY()
	TMap<FIntPoint, FSectorMeshes> StaticMeshMap;
	
	static TObjectPtr<UTerrainConfig> LoadTerrainConfig(const TCHAR* Path);
	static TObjectPtr<UMaterialInterface> LoadMaterial(const TCHAR* Path);
	static const FNoiseGroup* LoadNoiseGroup(const FString& Name, const TArray<FNoiseGroup>& NoiseGroupArray);

	void SetupSkyLightComponent();
	void SetupNoiseGeneration();

	TObjectPtr<USectorComponent> GenerateSector(const FIntPoint Coordinates);
	
	void GenerateSectorRenderData(const FIntPoint& Coordinates, const TObjectPtr<USectorComponent> Sector);
	
	uint8 ComputeBiomeIndex(const FVector2f WorldPosition) const;
	float SampleTerrainHeight(const FVector2f WorldPosition, const FNoiseGroup* NoiseGroup);

	FIntPoint GetPlayerSector() const;
	
	static TSet<FIntPoint> ComputeVisibleSet(const FIntPoint& PlayerSector, const TObjectPtr<UTerrainConfig> TerrainConfig);
	
	void AddMissingSectors(const TSet<FIntPoint>& VisibleSectorCoordinatesSet);
	void RemoveExpiredSectors(const TSet<FIntPoint>& VisibleSectorCoordinatesSet);
	void UpdateVisibleSectors();
};

#include "TerrainGenerator.h"
#include "Engine/TextureCube.h"
#include "Utility/StaticMeshConstructor.h"


ATerrainGenerator::ATerrainGenerator()
	:
	TerrainConfig { LoadTerrainConfig(TEXT("/Game/Terrain/DA_TerrainConfig.DA_TerrainConfig")) },
	GroundMaterial { LoadMaterial(TEXT("/Game/Terrain/M_Ground.M_Ground")) },
	WaterMaterial { LoadMaterial(TEXT("/Game/Terrain/M_Water.M_Water")) },
	BiomesMaterial { LoadMaterial(TEXT("/Game/Terrain/M_Biomes.M_Biomes")) },
	TerrainNoiseGroup { LoadNoiseGroup("Terrain", TerrainConfig->NoiseGroupArray) },
	WaterNoiseGroup { LoadNoiseGroup("Water", TerrainConfig->NoiseGroupArray) }
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->Mobility = EComponentMobility::Static;
	SetRootComponent(RootComponent);

	SetActorLocation(FVector::ZeroVector);

	SetupSkyLightComponent();
	SetupNoiseGeneration();
}

void ATerrainGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ATerrainGenerator::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(
		StreamingTimer,
		this,
		&ATerrainGenerator::UpdateVisibleSectors,
		0.25f,
		true
	);
}

TObjectPtr<UTerrainConfig> ATerrainGenerator::LoadTerrainConfig(const TCHAR* Path)
{
	if (
		const ConstructorHelpers::FObjectFinder<UTerrainConfig> ObjectFinder { Path };
		ObjectFinder.Succeeded()
	) {
		UTerrainConfig* TerrainConfig { ObjectFinder.Object };
		
		UE_LOG(LogTemp, Log, TEXT("Loaded: %s"), *TerrainConfig->GetName());

		return TerrainConfig;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Failed: %s"), Path);

	return nullptr;
}

TObjectPtr<UMaterialInterface> ATerrainGenerator::LoadMaterial(const TCHAR* Path)
{
	if (
		const ConstructorHelpers::FObjectFinder<UMaterialInterface> ObjectFinder { Path };
		ObjectFinder.Succeeded()
	) {
		TObjectPtr MaterialInterface { ObjectFinder.Object };
		UE_LOG(LogTemp, Log, TEXT("Loaded: %s"), *MaterialInterface->GetName());

		return MaterialInterface;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Failed: %s"), Path);

	return nullptr;
}

const FNoiseGroup* ATerrainGenerator::LoadNoiseGroup(const FString& Name, const TArray<FNoiseGroup>& NoiseGroupArray)
{
	for (const auto& NoiseGroup : NoiseGroupArray)
	{
		if (NoiseGroup.Name.Equals(Name, ESearchCase::IgnoreCase))
		{
			return &NoiseGroup;
		}
	}
	
	return nullptr;
}

void ATerrainGenerator::SetupSkyLightComponent()
{
	SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLightComponent->SetupAttachment(RootComponent);
	SkyLightComponent->SourceType = SLS_SpecifiedCubemap;

	const TCHAR* CubeMapPath { TEXT("/Engine/MapTemplates/Sky/DaylightAmbientCubemap") };
	
	if (
		const ConstructorHelpers::FObjectFinder<UTextureCube> ObjectFinder { CubeMapPath };
		ObjectFinder.Succeeded()
	) {
		SkyLightComponent->Cubemap = ObjectFinder.Object;
		SkyLightComponent->Intensity = 2.0f;

		UE_LOG(LogTemp, Log, TEXT("Loaded: %s"), *ObjectFinder.Object->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed: %s"), CubeMapPath);
	}
}

void ATerrainGenerator::SetupNoiseGeneration()
{
	const int32 TerrainSeed { TerrainConfig->Seed };
	const int32 BiomeSeed { TerrainConfig->Seed + 1 };
	
	TerrainNoise.SetSeed(TerrainSeed);
	TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	
	UE_LOG(LogTemp, Log, TEXT("Terrain Seed: %d"), TerrainSeed);
	
	const float BiomeFrequency { 1.0f / TerrainConfig->BiomePeriod };
	
	BiomeNoise.SetSeed(BiomeSeed);
	BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	BiomeNoise.SetFrequency(BiomeFrequency);

	UE_LOG(LogTemp, Log, TEXT("Biome Seed: %d"), BiomeSeed);
}

TObjectPtr<USectorComponent> ATerrainGenerator::GenerateSector(const FIntPoint Coordinates)
{
	if (TObjectPtr<USectorComponent>* SectorComponentPointer { ActiveSectorMap.Find(Coordinates) })
	{
		return *SectorComponentPointer;
	}
	
	const FVector WorldLocation {
		Coordinates.X * TerrainConfig->GetSectorSizeInCentimeters(),
		Coordinates.Y * TerrainConfig->GetSectorSizeInCentimeters(),
		0.0f,
	};

	const FName SectorName { *FString::Printf(TEXT("S_%d_%d"), Coordinates.X, Coordinates.Y) };

	TObjectPtr<USectorComponent> NewSectorComponent {
		NewObject<USectorComponent>(
			this,
			USectorComponent::StaticClass(),
			SectorName
		)
	};

	AddInstanceComponent(NewSectorComponent);
	
	NewSectorComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewSectorComponent->SetRelativeLocation(WorldLocation);
	NewSectorComponent->RegisterComponent();
	
	NewSectorComponent->Initialize(Coordinates, WorldLocation);

	ActiveSectorMap.Add(Coordinates, NewSectorComponent);

	return NewSectorComponent;
}

void ATerrainGenerator::GenerateSectorRenderData(const FIntPoint& Coordinates, const TObjectPtr<USectorComponent> Sector) {
	FSectorRenderData& RenderData { SectorRenderDataMap.FindOrAdd(Sector->Coordinates) };
	RenderData.Clear();

	RenderData.Coordinates = Coordinates;

    const int32 SectorSizeInCells { TerrainConfig->SectorSizeInCells };
    const float CellSizeInCentimeters { TerrainConfig->CellSizeInCentimeters };
    const int32 VertexCount { SectorSizeInCells + 1 };

	const float SectorWorldOffsetX { static_cast<float>(Sector->WorldLocation.X) };
	const float SectorWorldOffsetY { static_cast<float>(Sector->WorldLocation.Y) };

    for (int32 Y { 0 }; Y <= SectorSizeInCells; ++Y)
    {
        for (int32 X { 0 }; X <= SectorSizeInCells; ++X)
        {
        	const FVector2f LocalPosition { X * CellSizeInCentimeters, Y * CellSizeInCentimeters };
			const FVector2f WorldPosition { SectorWorldOffsetX + LocalPosition.X, SectorWorldOffsetY + LocalPosition.Y };
        	
        	const float GroundHeight { SampleTerrainHeight(WorldPosition, TerrainNoiseGroup) };

        	RenderData.Ground.VertexArray.Add({ LocalPosition.X, LocalPosition.Y, GroundHeight });

        	const FVector2f UV {
        		static_cast<float>(X) / static_cast<float>(SectorSizeInCells),
				static_cast<float>(Y) / static_cast<float>(SectorSizeInCells)
			};
        	
        	RenderData.Ground.UVArray.Add(UV);

        	const uint8 BiomeIndex { ComputeBiomeIndex(WorldPosition) };
        	
        	if (TerrainConfig->bDebugBiomes)
        	{
        		const FLinearColor DebugColor = TerrainConfig->BiomeDefinitionArray[BiomeIndex].DebugColor;
        		RenderData.Ground.VertexColorArray.Add(DebugColor);
        	}
        	else
        	{
        		const FVector2f NorthPosition { WorldPosition.X, WorldPosition.Y + CellSizeInCentimeters };
        		const FVector2f EastPosition { WorldPosition.X + CellSizeInCentimeters, WorldPosition.Y };
        		const FVector2f SouthPosition { WorldPosition.X, WorldPosition.Y - CellSizeInCentimeters };
        		const FVector2f WestPosition { WorldPosition.X - CellSizeInCentimeters, WorldPosition.Y };
        		
        		const int32 BiomeIndexSum {
        			BiomeIndex + 
					ComputeBiomeIndex(NorthPosition) + 
					ComputeBiomeIndex(EastPosition) + 
					ComputeBiomeIndex(SouthPosition) + 
					ComputeBiomeIndex(WestPosition)
				};

        		const float BiomeIndexAverage { BiomeIndexSum / 5.0f };
        		const float BiomeIndexMax { static_cast<float>(TerrainConfig->BiomeDefinitionArray.Num() - 1) };
        		
        		const float BiomeIndexAverageNormalized { BiomeIndexAverage / BiomeIndexMax };

        		const FLinearColor VertexColor = {
        			BiomeIndexAverageNormalized, 
					0.f, 
					0.f, 
					1.f
				};
        		
        		RenderData.Ground.VertexColorArray.Add(VertexColor);
        	}

        	const float WaterHeight {
				SampleTerrainHeight(WorldPosition, WaterNoiseGroup) +
				TerrainConfig->WaterLevel
        	};

        	RenderData.Water.VertexArray.Add({ LocalPosition.X, LocalPosition.Y, WaterHeight });
        	RenderData.Water.UVArray.Add(UV);
        	RenderData.Water.VertexColorArray.Add(FLinearColor::White);
        }
    }

    for (int32 Y { 0 }; Y < SectorSizeInCells; ++Y)
    {
        for (int32 X { 0 }; X < SectorSizeInCells; ++X)
        {
            const int32 Index0 { Y * VertexCount + X };
            const int32 Index1 { Y * VertexCount + (X + 1) };
            const int32 Index2 { (Y + 1) * VertexCount + X };
            const int32 Index3 { (Y + 1) * VertexCount + (X + 1) };

            RenderData.Ground.IndexArray.Append({ Index0, Index2, Index3, Index0, Index3, Index1 });
            RenderData.Water.IndexArray.Append({ Index0, Index2, Index3, Index0, Index3, Index1 });
        }
    }
}

uint8 ATerrainGenerator::ComputeBiomeIndex(const FVector2f WorldPosition) const
{
	const int32 BiomeCount { TerrainConfig->BiomeDefinitionArray.Num() };

	if (BiomeCount == 0)
	{
		return 0;
	}
	
	constexpr float NoiseScale { 1.0f };
	
	const float NoiseValue { 
		BiomeNoise.GetNoise(
			NoiseScale * WorldPosition.X, 
			NoiseScale * WorldPosition.Y
		) 
	};
	
	UE_LOG(LogTemp, Warning, TEXT("Noise: %f"), NoiseValue);
	
	const float NoiseNormalized { (NoiseValue + 1.0f) * 0.5f };
	
	const int32 BiomeIndexRaw { FMath::FloorToInt(BiomeCount * NoiseNormalized) };
	const int32 BiomeIndexClamped { FMath::Clamp(BiomeIndexRaw, 0, BiomeCount - 1) };

	return static_cast<uint8>(BiomeIndexClamped);
}

float ATerrainGenerator::SampleTerrainHeight(const FVector2f WorldPosition,  const FNoiseGroup* NoiseGroup)
{
	float NoiseValue { 0.0f };

	for (const auto& [Weight, Period, Amplitude] : NoiseGroup->NoiseLayerArray)
	{
		const float Frequency { 1.0f / Period };
		
		TerrainNoise.SetFrequency(Frequency);

		constexpr float XScale { 1.01f };
		constexpr float XOffset { 17.123f };
		
		constexpr float YScale { 0.99f };
		constexpr float YOffset { 43.512f };
		
		const float LayerNoiseValue {
			Amplitude *
			TerrainNoise.GetNoise(
				WorldPosition.X * XScale + XOffset, WorldPosition.Y * YScale + YOffset
			)
		};
	
		NoiseValue += Weight * LayerNoiseValue;
	}

	return NoiseValue;
}

void ATerrainGenerator::UpdateVisibleSectors()
{
	const FIntPoint PlayerSectorCoordinates { GetPlayerSector() };
	const TSet VisibleSectorCoordinatesSet { ComputeVisibleSet(PlayerSectorCoordinates, TerrainConfig) };

	AddMissingSectors(VisibleSectorCoordinatesSet);
	RemoveExpiredSectors(VisibleSectorCoordinatesSet);
}

FIntPoint ATerrainGenerator::GetPlayerSector() const
{
	const APawn* PlayerPawn { GetWorld()->GetFirstPlayerController()->GetPawn() };
	const FVector Location { PlayerPawn->GetActorLocation() };

	const FIntPoint SectorPosition {
		FMath::FloorToInt(Location.X / TerrainConfig->GetSectorSizeInCentimeters()),
		FMath::FloorToInt(Location.Y / TerrainConfig->GetSectorSizeInCentimeters())
	};

	return SectorPosition;
}

TSet<FIntPoint> ATerrainGenerator::ComputeVisibleSet(const FIntPoint& PlayerSector, const TObjectPtr<UTerrainConfig> TerrainConfig)
{
	constexpr int32 ViewRadius { 1 };

	TSet<FIntPoint> VisibleSectorCoordinates;

	for (int X { -ViewRadius }; X <= ViewRadius; ++X)
	{
		for (int Y { -ViewRadius }; Y <= ViewRadius; ++Y)
		{
			if (
				const FIntPoint SectorCoordinates { PlayerSector + FIntPoint(X, Y) };
				SectorCoordinates.X >= 0 &&
				SectorCoordinates.Y >= 0 &&
				SectorCoordinates.X < TerrainConfig->WorldSizeInSectors &&
				SectorCoordinates.Y < TerrainConfig->WorldSizeInSectors
			) {
				VisibleSectorCoordinates.Add(SectorCoordinates);
			}
		}
	}

	return VisibleSectorCoordinates;
}

void ATerrainGenerator::AddMissingSectors(const TSet<FIntPoint>& VisibleSectorCoordinatesSet)
{
	for (const FIntPoint& SectorCoordinates : VisibleSectorCoordinatesSet)
	{
		TObjectPtr<USectorComponent> SectorComponent;

		if (const TObjectPtr<USectorComponent>* SectorComponentPtr { ActiveSectorMap.Find(SectorCoordinates) })
		{
			SectorComponent = *SectorComponentPtr;            
		}
		else
		{
			SectorComponent = GenerateSector(SectorCoordinates);
			
			ActiveSectorMap.Add(SectorCoordinates, SectorComponent);
		}

		if (!SectorRenderDataMap.Contains(SectorCoordinates))
		{
			GenerateSectorRenderData(SectorCoordinates, SectorComponent);
		}

		if (!StaticMeshMap.Contains(SectorCoordinates))
		{
			FSectorMeshes SectorMeshes {
				FStaticMeshConstructor::Run(
					this,
					*FString::Printf(TEXT("SMG_%d_%d"), SectorCoordinates.X, SectorCoordinates.Y),
					SectorRenderDataMap[SectorCoordinates].Ground,
					true
				),
				FStaticMeshConstructor::Run(
					this,
					*FString::Printf(TEXT("SMW_%d_%d"), SectorCoordinates.X, SectorCoordinates.Y),
					SectorRenderDataMap[SectorCoordinates].Water,
					false
				)
			};
			
			StaticMeshMap.Add(SectorCoordinates, SectorMeshes);
		}
		
		UMaterialInterface* ActiveGroundMaterial;
		
		if (TerrainConfig->bDebugBiomes)
		{
			ActiveGroundMaterial = BiomesMaterial;
		}
		else
		{
			ActiveGroundMaterial = GroundMaterial;
		}

		SectorComponent->GroundStaticMeshComponent->SetStaticMesh(StaticMeshMap[SectorCoordinates].GroundStaticMesh);
		SectorComponent->GroundStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
		SectorComponent->GroundStaticMeshComponent->SetMaterial(0, ActiveGroundMaterial);
		SectorComponent->GroundStaticMeshComponent->MarkRenderStateDirty();
		
		SectorComponent->WaterStaticMeshComponent->SetStaticMesh(StaticMeshMap[SectorCoordinates].WaterStaticMesh);
		SectorComponent->WaterStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
		SectorComponent->WaterStaticMeshComponent->SetMaterial(0, WaterMaterial);
		SectorComponent->WaterStaticMeshComponent->SetTranslucentSortPriority(1);
		SectorComponent->WaterStaticMeshComponent->SetCastShadow(false);
		SectorComponent->WaterStaticMeshComponent->SetReceivesDecals(false);
		SectorComponent->WaterStaticMeshComponent->MarkRenderStateDirty();
	}
}

void ATerrainGenerator::RemoveExpiredSectors(const TSet<FIntPoint>& VisibleSectorCoordinatesSet)
{
	for (auto Iterator { ActiveSectorMap.CreateIterator() }; Iterator; ++Iterator)
	{
		if (!VisibleSectorCoordinatesSet.Contains(Iterator.Key()))
		{
			USectorComponent* SectorComponent { Iterator.Value() };
			
			if (SectorComponent->GroundStaticMeshComponent)
			{
				SectorComponent->GroundStaticMeshComponent->DestroyComponent();
			}
			
			if (SectorComponent->WaterStaticMeshComponent)
			{
				SectorComponent->WaterStaticMeshComponent->DestroyComponent();
			}
			
			SectorComponent->DestroyComponent();
			
			Iterator.RemoveCurrent();
		}
	}
}

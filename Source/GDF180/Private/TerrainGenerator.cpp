#include "TerrainGenerator.h"
#include "Engine/TextureCube.h"
#include "Utility/StaticMeshConstructor.h"

const FIntPoint ATerrainGenerator::NeighborOffsetArray[8] {
	{ -1,  0 }, {  1,  0 }, 
	{  0, -1 }, {  0,  1 },
	{ -1, -1 }, {  1, -1 }, 
	{ -1,  1 }, {  1,  1 }
};

ATerrainGenerator::ATerrainGenerator()
	:
	TerrainConfig { LoadTerrainConfig(TEXT("/Game/Terrain/DA_TerrainConfig.DA_TerrainConfig")) },
	BiomeSet { LoadBiomeSet(TEXT("/Game/Terrain/DA_BiomeSet.DA_BiomeSet")) },
	TerrainMaterial { LoadMaterial(TEXT("/Game/Terrain/M_Terrain.M_Terrain")) },
	WaterMaterial { LoadMaterial(TEXT("/Game/Terrain/M_Water.M_Water")) },
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

TObjectPtr<UBiomeSet> ATerrainGenerator::LoadBiomeSet(const TCHAR* Path)
{
	if (
		const ConstructorHelpers::FObjectFinder<UBiomeSet> ObjectFinder { Path };
		ObjectFinder.Succeeded()
	) {
		UBiomeSet* BiomeSet { ObjectFinder.Object };
		
		UE_LOG(LogTemp, Log, TEXT("Loaded: %s"), *BiomeSet->GetName());

		return BiomeSet;
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
	
	UE_LOG(LogTemp, Log, TEXT("Terrain Seed: %d"), TerrainSeed);
	UE_LOG(LogTemp, Log, TEXT("Biome Seed: %d"), BiomeSeed);
	
	TerrainNoise.SetSeed(TerrainSeed);
	TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	
	BiomeNoise.SetSeed(BiomeSeed);
	BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	BiomeNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
	BiomeNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
	BiomeNoise.SetFrequency(BiomeSet->GetFrequency());
}

TObjectPtr<USectorComponent> ATerrainGenerator::GenerateSector(const FIntPoint SectorCoordinates)
{
	if (TObjectPtr<USectorComponent>* SectorComponentPointer { ActiveSectorMap.Find(SectorCoordinates) })
	{
		return *SectorComponentPointer;
	}
	
	const FName SectorName { *FString::Printf(TEXT("S_%d_%d"), SectorCoordinates.X, SectorCoordinates.Y) };
	
	const FVector3f WorldLocation { SectorCoordinates * TerrainConfig->GetSectorSizeInCentimeters() };

	TObjectPtr<USectorComponent> NewSectorComponent {
		NewObject<USectorComponent>(
			this,
			USectorComponent::StaticClass(),
			SectorName
		)
	};

	AddInstanceComponent(NewSectorComponent.Get());
	
	NewSectorComponent->AttachToComponent(RootComponent.Get(), FAttachmentTransformRules::KeepRelativeTransform);
	NewSectorComponent->SetRelativeLocation(FVector { WorldLocation });
	NewSectorComponent->RegisterComponent();
	NewSectorComponent->Initialize(SectorCoordinates, WorldLocation);

	ActiveSectorMap.Add(NewSectorComponent->SectorCoordinates, NewSectorComponent);

	return NewSectorComponent;
}

void ATerrainGenerator::GenerateSectorRenderData(const TObjectPtr<USectorComponent> SectorComponent) {
	FSectorRenderData& SectorRenderData { SectorRenderDataMap.FindOrAdd(SectorComponent->SectorCoordinates) };
	SectorRenderData.Clear();

	SectorRenderData.SectorCoordinates = SectorComponent->SectorCoordinates;

	int32 IndexBase { 0 };

	const FVector2f SectorWorldPosition { SectorComponent->WorldLocation.X, SectorComponent->WorldLocation.Y };

	for (int32 Y { 0 }; Y < TerrainConfig->SectorSizeInCells; ++Y)
	{
	    for (int32 X { 0 }; X < TerrainConfig->SectorSizeInCells; ++X)
	    {
	        FVector2f VertexPosition00 { X * TerrainConfig->CellSizeInCentimeters, Y * TerrainConfig->CellSizeInCentimeters };
	        FVector2f VertexPosition10 { (X + 1) * TerrainConfig->CellSizeInCentimeters, Y * TerrainConfig->CellSizeInCentimeters };
	        FVector2f VertexPosition11 { (X + 1) * TerrainConfig->CellSizeInCentimeters, ( Y+ 1) * TerrainConfig->CellSizeInCentimeters };
	        FVector2f VertexPosition01 { X * TerrainConfig->CellSizeInCentimeters, (Y + 1) * TerrainConfig->CellSizeInCentimeters };

	        FVector2f CellWorldPosition { 
	        	SectorWorldPosition + FVector2f{
		            (X + 0.5f) * TerrainConfig->CellSizeInCentimeters,
		            (Y + 0.5f) * TerrainConfig->CellSizeInCentimeters
		        }
	        };

	        const uint8 BiomeIndex { SampleBiomeIndex(CellWorldPosition) };
	        const float BiomeIndexMax { BiomeSet->BiomeDefinitionArray.Num() - 1.0f };
	    	
	        const float EncodedBiomeIndex { 
	        	BiomeIndexMax > 0.0f ? static_cast<float>(BiomeIndex) / BiomeIndexMax : 0.0f
	        };

	        FVector4f VertexColor { EncodedBiomeIndex, 0, 0, 1 };

	        auto AddVertex = [&](const FVector2f& LocalPosition)
	        {
	            const FVector2f WorldPosition { SectorWorldPosition + LocalPosition };
	            const float TerrainHeight { SampleHeight(WorldPosition, TerrainNoiseGroup) };
	        	const float WaterHeight { SampleHeight(WorldPosition, WaterNoiseGroup) };
	        	
	        	const FVector3f TerrainVertexPosition { LocalPosition.X, LocalPosition.Y, TerrainHeight };
	        	const FVector3f WaterVertexPosition { LocalPosition.X, LocalPosition.Y, WaterHeight };

	            const FVector2f UV {
	                LocalPosition.X / (TerrainConfig->SectorSizeInCells * TerrainConfig->CellSizeInCentimeters),
	                LocalPosition.Y / (TerrainConfig->SectorSizeInCells * TerrainConfig->CellSizeInCentimeters)
	            };

	            SectorRenderData.GroundMeshRenderData.VertexArray.Add(TerrainVertexPosition);
	            SectorRenderData.GroundMeshRenderData.UVArray.Add(UV);
	            SectorRenderData.GroundMeshRenderData.VertexColorArray.Add(VertexColor);
	        	
	        	SectorRenderData.WaterMeshRenderData.VertexArray.Add(WaterVertexPosition);
	        	SectorRenderData.WaterMeshRenderData.UVArray.Add(UV);
	        	SectorRenderData.WaterMeshRenderData.VertexColorArray.Add(VertexColor);
	        };

	        int32 Index0 { IndexBase + 0 };
	        int32 Index1 { IndexBase + 1 };
	        int32 Index2 { IndexBase + 2 };
	        int32 Index3 { IndexBase + 3 };

	        AddVertex(VertexPosition00);
	        AddVertex(VertexPosition10);
	        AddVertex(VertexPosition11);
	        AddVertex(VertexPosition01);

	        SectorRenderData.GroundMeshRenderData.IndexArray.Append({
	            Index0, Index2, Index1,
	            Index0, Index3, Index2
	        });
	    	
	    	SectorRenderData.WaterMeshRenderData.IndexArray.Append({
				Index0, Index2, Index1,
				Index0, Index3, Index2
			});

	        IndexBase += 4;
	    }
	}
}

float ATerrainGenerator::SampleHeight(const FVector2f WorldPosition, const FNoiseGroup* NoiseGroup)
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
	const TObjectPtr PlayerPawn { GetWorld()->GetFirstPlayerController()->GetPawn() };
	const FVector Location { PlayerPawn->GetActorLocation() };

	const FIntPoint SectorPosition {
		FMath::FloorToInt(Location.X / TerrainConfig->GetSectorSizeInCentimeters()),
		FMath::FloorToInt(Location.Y / TerrainConfig->GetSectorSizeInCentimeters())
	};

	return SectorPosition;
}

TSet<FIntPoint> ATerrainGenerator::ComputeVisibleSet(const FIntPoint& PlayerSector, const TObjectPtr<UTerrainConfig> TerrainConfig)
{
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

		if (const TObjectPtr<USectorComponent>* FindResult { ActiveSectorMap.Find(SectorCoordinates) })
		{
			SectorComponent = *FindResult;      
		}
		else
		{
			SectorComponent = GenerateSector(SectorCoordinates);
			
			ActiveSectorMap.Add(SectorComponent->SectorCoordinates, SectorComponent);
		}
		
		if (!SectorRenderDataMap.Contains(SectorComponent->SectorCoordinates))
		{
			GenerateSectorRenderData(SectorComponent);
		}
		
		if (!StaticMeshMap.Contains(SectorComponent->SectorCoordinates))
		{
			FSectorRenderData& SectorRenderData { SectorRenderDataMap[SectorComponent->SectorCoordinates] };
			
			FSectorMeshes SectorMeshes {
				FStaticMeshConstructor::Run(
					this,
					*FString::Printf(TEXT("SMG_%d_%d"), SectorComponent->SectorCoordinates.X, SectorComponent->SectorCoordinates.Y),
					SectorRenderData.GroundMeshRenderData,
					true
				),
				FStaticMeshConstructor::Run(
					this,
					*FString::Printf(TEXT("SMW_%d_%d"), SectorComponent->SectorCoordinates.X, SectorComponent->SectorCoordinates.Y),
					SectorRenderData.WaterMeshRenderData,
					false
				)
			};
			
			StaticMeshMap.Add(SectorComponent->SectorCoordinates, SectorMeshes);
		}
		
		const auto& [GroundStaticMesh, WaterStaticMesh]
		{
			StaticMeshMap[SectorComponent->SectorCoordinates]
		};
		
		SectorComponent->GroundStaticMeshComponent->SetStaticMesh(GroundStaticMesh.Get());
		SectorComponent->GroundStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
		SectorComponent->GroundStaticMeshComponent->SetMaterial(0, TerrainMaterial.Get());
		SectorComponent->GroundStaticMeshComponent->MarkRenderStateDirty();
		
		UMaterialInstanceDynamic* TerrainMaterialInstance 
		{ 
			SectorComponent->GroundStaticMeshComponent->CreateDynamicMaterialInstance(0) 
		};
		
		TerrainMaterialInstance->SetScalarParameterValue(TEXT("BiomeIndexMax"), BiomeSet->BiomeDefinitionArray.Num() - 1);
		
		SectorComponent->WaterStaticMeshComponent->SetStaticMesh(WaterStaticMesh.Get());
		SectorComponent->WaterStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
		SectorComponent->WaterStaticMeshComponent->SetMaterial(0, WaterMaterial.Get());
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

uint8 ATerrainGenerator::SampleBiomeIndex(const FVector2f& WorldPosition) const
{
	const int32 BiomeCount { BiomeSet->BiomeDefinitionArray.Num() };
	
	const float VoronoiLabel { BiomeNoise.GetNoise(WorldPosition.X, WorldPosition.Y) };
	const float VoronoiLabelNormalized { 0.5f * (VoronoiLabel + 1.0f) };
	
	const uint8 BiomeIndex { 
		static_cast<uint8> (
			FMath::Clamp(
				FMath::FloorToInt(VoronoiLabelNormalized * BiomeCount),
				0,
				BiomeCount - 1
			)
		)
	};
	
	return BiomeIndex;
}

int32 ATerrainGenerator::GetVertexIndex(const FIntPoint GridPosition) const
{
    const int32 VerticesPerRow { TerrainConfig->SectorSizeInCells + 1 };
	
	return GridPosition.Y * VerticesPerRow + GridPosition.X;
}

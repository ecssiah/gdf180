#include "TerrainGenerator.h"

#include "GPUSkinVertexFactory.h"
#include "Engine/TextureCube.h"
#include "Utility/StaticMeshConstructor.h"

const FIntPoint ATerrainGenerator::NeighborOffsetArray[4] = {
	{ -1,  0 },
	{  1,  0 },
	{  0, -1 },
	{  0,  1 },
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
	
	TerrainNoise.SetSeed(TerrainSeed);
	TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	
	UE_LOG(LogTemp, Log, TEXT("Terrain Seed: %d"), TerrainSeed);
	
	const float BiomeFrequency { 1.0f / TerrainConfig->BiomePeriod };
	
	BiomeNoise.SetSeed(BiomeSeed);
	BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	BiomeNoise.SetFrequency(BiomeFrequency);

	UE_LOG(LogTemp, Log, TEXT("Biome Seed: %d"), BiomeSeed);
}

TObjectPtr<USectorComponent> ATerrainGenerator::GenerateSector(const FIntPoint SectorCoordinates)
{
	if (TObjectPtr<USectorComponent>* SectorComponentPointer { ActiveSectorMap.Find(SectorCoordinates) })
	{
		return *SectorComponentPointer;
	}
	
	const FVector3f WorldLocation { SectorCoordinates * TerrainConfig->GetSectorSizeInCentimeters() };

	const FName SectorName { *FString::Printf(TEXT("S_%d_%d"), SectorCoordinates.X, SectorCoordinates.Y) };

	TObjectPtr<USectorComponent> NewSectorComponent {
		NewObject<USectorComponent>(
			this,
			USectorComponent::StaticClass(),
			SectorName
		)
	};

	AddInstanceComponent(NewSectorComponent);
	
	NewSectorComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewSectorComponent->SetRelativeLocation(FVector { WorldLocation });
	NewSectorComponent->RegisterComponent();
	NewSectorComponent->Initialize(SectorCoordinates, WorldLocation);

	ActiveSectorMap.Add(SectorCoordinates, NewSectorComponent);

	return NewSectorComponent;
}

void ATerrainGenerator::GenerateSectorRenderData(const TObjectPtr<USectorComponent> SectorComponent) {
	FSectorRenderData& SectorRenderData { SectorRenderDataMap.FindOrAdd(SectorComponent->SectorCoordinates) };
	SectorRenderData.Clear();

	SectorRenderData.SectorCoordinates = SectorComponent->SectorCoordinates;

	const FVector2f SectorWorldOffset { SectorComponent->WorldLocation.X, SectorComponent->WorldLocation.Y };

    for (int32 Y { 0 }; Y <= TerrainConfig->SectorSizeInCells; ++Y)
    {
        for (int32 X { 0 }; X <= TerrainConfig->SectorSizeInCells; ++X)
        {
        	const FVector2f LocalPosition { X * TerrainConfig->CellSizeInCentimeters, Y * TerrainConfig->CellSizeInCentimeters };
			const FVector2f WorldPosition { SectorWorldOffset + LocalPosition };
        	
        	const float TerrainHeight { SampleHeight(WorldPosition, TerrainNoiseGroup) };
        	const FVector3f TerrainVertexPosition { LocalPosition.X, LocalPosition.Y, TerrainHeight };
        	
        	SectorRenderData.GroundMeshRenderData.VertexArray.Add(TerrainVertexPosition);

        	const float WaterHeight { SampleHeight(WorldPosition, WaterNoiseGroup) + TerrainConfig->WaterLevel };
        	const FVector3f WaterVertexPosition { LocalPosition.X, LocalPosition.Y, WaterHeight };

        	SectorRenderData.WaterMeshRenderData.VertexArray.Add(WaterVertexPosition);

        	const uint8 BiomeIndex { SampleBiomeIndex(WorldPosition) };
        	
        	SectorRenderData.GroundMeshRenderData.PrimaryBiomeIndexArray.Add(BiomeIndex);
        	
        	const FVector2f UV {
        		static_cast<float>(X) / static_cast<float>(TerrainConfig->SectorSizeInCells),
				static_cast<float>(Y) / static_cast<float>(TerrainConfig->SectorSizeInCells)
			};
        	
        	SectorRenderData.GroundMeshRenderData.UVArray.Add(UV);
        	SectorRenderData.WaterMeshRenderData.UVArray.Add(UV);
        	
        	SectorRenderData.WaterMeshRenderData.VertexColorArray.Add(FLinearColor::White);
        }
    }

    for (int32 Y { 0 }; Y < TerrainConfig->SectorSizeInCells; ++Y)
    {
        for (int32 X { 0 }; X < TerrainConfig->SectorSizeInCells; ++X)
        {
        	const FIntPoint Vertex0GridPosition {X, Y};
        	const FIntPoint Vertex1GridPosition {X + 1, Y};
        	const FIntPoint Vertex2GridPosition {X, Y + 1};
        	const FIntPoint Vertex3GridPosition {X + 1, Y + 1};
        	
        	const int32 Vertex0Index { GetVertexIndex(Vertex0GridPosition) };
        	const int32 Vertex1Index { GetVertexIndex(Vertex1GridPosition) };
        	const int32 Vertex2Index { GetVertexIndex(Vertex2GridPosition) };
        	const int32 Vertex3Index { GetVertexIndex(Vertex3GridPosition) };

            SectorRenderData.GroundMeshRenderData.IndexArray.Append({ 
            	Vertex0Index, 
            	Vertex2Index, 
            	Vertex3Index, 
            	Vertex0Index, 
            	Vertex3Index, 
            	Vertex1Index 
			});
			
            SectorRenderData.WaterMeshRenderData.IndexArray.Append({ 
            	Vertex0Index, 
            	Vertex2Index, 
            	Vertex3Index, 
            	Vertex0Index, 
            	Vertex3Index, 
            	Vertex1Index 
			});
        }
    }
	
	const int32 BiomeIndexNum { SectorRenderData.GroundMeshRenderData.PrimaryBiomeIndexArray.Num() };
	const float BiomeIndexMax { static_cast<float>(BiomeSet->BiomeDefinitionArray.Num() - 1) };
	
	SectorRenderData.GroundMeshRenderData.BoundaryMaskArray.SetNum(BiomeIndexNum);
	SectorRenderData.GroundMeshRenderData.SecondaryBiomeIndexArray.SetNum(BiomeIndexNum);
	SectorRenderData.GroundMeshRenderData.VertexColorArray.SetNum(BiomeIndexNum);
	
	for (int32 Y { 0 }; Y <= TerrainConfig->SectorSizeInCells; ++Y)
	{
		for (int32 X { 0 }; X <= TerrainConfig->SectorSizeInCells; ++X)
		{
			const FIntPoint GridPosition { X, Y };
			const int32 VertexIndex = GetVertexIndex(GridPosition);
			
			const auto [bIsBoundary, SecondaryBiomeIndex] = GetSecondaryBiomeIndex(GridPosition, SectorComponent);
			
			SectorRenderData.GroundMeshRenderData.BoundaryMaskArray[VertexIndex] = bIsBoundary ? 1 : 0;
			SectorRenderData.GroundMeshRenderData.SecondaryBiomeIndexArray[VertexIndex] = SecondaryBiomeIndex;
		}
	}
	
	for (int32 BiomeIndex { 0 }; BiomeIndex < BiomeIndexNum; ++BiomeIndex)
	{
		const uint8 PrimaryBiomeIndex { SectorRenderData.GroundMeshRenderData.PrimaryBiomeIndexArray[BiomeIndex] };
		const uint8 SecondaryBiomeIndex { SectorRenderData.GroundMeshRenderData.SecondaryBiomeIndexArray[BiomeIndex] };
		
		const float PrimaryBiomeIndexNormalized { PrimaryBiomeIndex / BiomeIndexMax };
		const float SecondaryBiomeIndexNormalized { SecondaryBiomeIndex / BiomeIndexMax };
		
		const int32 BoundaryMask { SectorRenderData.GroundMeshRenderData.BoundaryMaskArray[BiomeIndex] };

		const FVector4f VertexColor {
			PrimaryBiomeIndexNormalized,
			SecondaryBiomeIndexNormalized,
			BoundaryMask == 1 ? 1.0f : 0.0f,
			1.0f
		};
		
		SectorRenderData.GroundMeshRenderData.VertexColorArray[BiomeIndex] = VertexColor;
	}
}

float ATerrainGenerator::SampleHeight(const FVector2f WorldPosition,  const FNoiseGroup* NoiseGroup)
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
			FSectorRenderData& SectorRenderData = SectorRenderDataMap[SectorComponent->SectorCoordinates];
			
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
		
		const auto& [GroundStaticMesh, WaterStaticMesh] = StaticMeshMap[SectorComponent->SectorCoordinates];
		
		SectorComponent->GroundStaticMeshComponent->SetStaticMesh(GroundStaticMesh);
		SectorComponent->GroundStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
		SectorComponent->GroundStaticMeshComponent->SetMaterial(0, TerrainMaterial);
		SectorComponent->GroundStaticMeshComponent->MarkRenderStateDirty();
		
		UMaterialInstanceDynamic* TerrainMaterialInstance = SectorComponent->GroundStaticMeshComponent->CreateDynamicMaterialInstance(0);
		TerrainMaterialInstance->SetScalarParameterValue(TEXT("BiomeIndexMax"), BiomeSet->BiomeDefinitionArray.Num() - 1);
		
		SectorComponent->WaterStaticMeshComponent->SetStaticMesh(WaterStaticMesh);
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

uint8 ATerrainGenerator::SampleBiomeIndex(const FVector2f& WorldPosition) const
{
	const int32 BiomeCount { BiomeSet->BiomeDefinitionArray.Num() };
	
	if (BiomeCount == 0)
	{
		return 0;
	}

	const float NoiseValue { BiomeNoise.GetNoise(WorldPosition.X, WorldPosition.Y) };
	const float NoiseNormalized { (NoiseValue + 1.0f) * 0.5f };

	const int32 BiomeIndex { FMath::Clamp(
		FMath::FloorToInt(NoiseNormalized * BiomeCount),
		0,
		BiomeCount - 1)
	};

	return static_cast<uint8>(BiomeIndex);
}

int32 ATerrainGenerator::GetVertexIndex(const FIntPoint GridPosition) const
{
    const int32 VertexCount { TerrainConfig->SectorSizeInCells + 1 };
	
	return GridPosition.Y * VertexCount + GridPosition.X;
}

std::tuple<bool, uint8> ATerrainGenerator::GetSecondaryBiomeIndex(const FIntPoint GridPosition, const TObjectPtr<USectorComponent> SectorComponent)
{
	FSectorRenderData& SectorRenderData { SectorRenderDataMap.FindOrAdd(SectorComponent->SectorCoordinates) };
	
	const int32 VertexIndex = GetVertexIndex(GridPosition);
	const uint8 PrimaryBiomeIndex = SectorRenderData.GroundMeshRenderData.PrimaryBiomeIndexArray[VertexIndex];
	
	for (const auto& NeighborOffset: NeighborOffsetArray)
	{
		const FIntPoint NeighborPosition { GridPosition + NeighborOffset };
		
		if (
			NeighborPosition.X < 0 || 
			NeighborPosition.Y < 0 || 
			NeighborPosition.X > TerrainConfig->SectorSizeInCells || 
			NeighborPosition.Y > TerrainConfig->SectorSizeInCells)
		{
			continue;
		}
		
		const int32 NeighborVertexIndex = GetVertexIndex(NeighborPosition);
		
		if (
			const uint8 NeighborBiomeIndex = SectorRenderData.GroundMeshRenderData.PrimaryBiomeIndexArray[NeighborVertexIndex]; 
			NeighborBiomeIndex != PrimaryBiomeIndex)
		{
			return { true, NeighborBiomeIndex };
		}
	}
	
	return { false, PrimaryBiomeIndex };
}

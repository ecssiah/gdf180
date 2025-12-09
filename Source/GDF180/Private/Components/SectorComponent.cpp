#include "SectorComponent.h"


USectorComponent::USectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetMobility(EComponentMobility::Movable);
	
	GroundStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundMesh"));
	GroundStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	GroundStaticMeshComponent->SetupAttachment(this);
	GroundStaticMeshComponent->SetMobility(EComponentMobility::Movable);

	WaterStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
	WaterStaticMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	WaterStaticMeshComponent->SetupAttachment(this);
	WaterStaticMeshComponent->SetMobility(EComponentMobility::Movable);
}

void USectorComponent::Initialize(const FIntPoint& InSectorCoordinates, const FVector3f& InWorldLocation)
{
	SectorCoordinates = InSectorCoordinates;
	WorldLocation = InWorldLocation;
}

void USectorComponent::OnRegister()
{
	Super::OnRegister();

	GroundStaticMeshComponent->AttachToComponent(
		this,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	WaterStaticMeshComponent->AttachToComponent(
		this,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	GroundStaticMeshComponent->RegisterComponent();
	WaterStaticMeshComponent->RegisterComponent();
}
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SectorComponent.generated.h"


UCLASS(ClassGroup=(Custom))
class USectorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USectorComponent();

	FIntPoint Coordinates;
	FVector WorldLocation;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> GroundStaticMeshComponent;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> WaterStaticMeshComponent;
	
	void Initialize(const FIntPoint& InCoordinates, const FVector& InWorldLocation);
	
protected:
	virtual void OnRegister() override;
};
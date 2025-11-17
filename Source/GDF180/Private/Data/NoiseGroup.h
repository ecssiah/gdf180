#pragma once

#include "NoiseLayer.h"
#include "NoiseGroup.generated.h"


USTRUCT(BlueprintType)
struct FNoiseGroup
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNoiseLayer> NoiseLayerArray;
};

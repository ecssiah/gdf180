#pragma once

#include "RingDefinition.generated.h"


USTRUCT(BlueprintType)
struct FRingDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere)
	float InnerRadius { 0.0f };

	UPROPERTY(EditAnywhere)
	float OuterRadius { 0.0f };

	UPROPERTY(EditAnywhere)
	TMap<uint8, float> BiomeWeightMap;
};

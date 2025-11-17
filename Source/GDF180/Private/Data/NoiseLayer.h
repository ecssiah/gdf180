#pragma once

#include "NoiseLayer.generated.h"


USTRUCT(BlueprintType)
struct FNoiseLayer
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Weight { 1.0f };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Period { 100.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Amplitude { 100.0f };
};

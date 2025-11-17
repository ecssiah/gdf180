#pragma once

#include "BiomeDefinition.generated.h"


USTRUCT(BlueprintType)
struct FBiomeDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Name;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor DebugColor { FLinearColor::Black };
};

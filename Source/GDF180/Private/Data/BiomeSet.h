#pragma once

#include "CoreMinimal.h"
#include "BiomeDefinition.h"
#include "RingDefinition.h"
#include "Engine/DataAsset.h"
#include "BiomeSet.generated.h"

UCLASS(BlueprintType)
class UBiomeSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UBiomeSet();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDebugBiomes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BiomePeriod;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBiomeDefinition> BiomeDefinitionArray;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRingDefinition> RingDefinitionArray;
	
	float GetFrequency() const;
	
	const FRingDefinition& GetRingDefinition(const float Radius) const;
};

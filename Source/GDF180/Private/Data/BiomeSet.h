#pragma once

#include "CoreMinimal.h"
#include "BiomeDefinition.h"
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
	
	float GetFrequency() const;
};

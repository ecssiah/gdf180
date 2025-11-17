#pragma once

#include "SectorMeshes.generated.h"


USTRUCT()
struct FSectorMeshes
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UStaticMesh> GroundStaticMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> WaterStaticMesh;
};

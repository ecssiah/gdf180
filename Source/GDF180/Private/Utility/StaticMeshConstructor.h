#pragma once

#include "CoreMinimal.h"
#include "../Data/MeshRenderData.h"


class UStaticMesh;
struct FSectorRenderData;

struct FStaticMeshConstructor
{
	static TObjectPtr<UStaticMesh> Run(
		UObject* Outer,
		const TCHAR* MeshName,
		const FMeshRenderData& MeshRenderData,
		const bool bGenerateCollision = false
	);
};
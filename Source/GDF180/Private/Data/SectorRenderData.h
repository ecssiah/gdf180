#pragma once

#include "CoreMinimal.h"
#include "MeshRenderData.h"


struct FSectorRenderData
{
	FIntPoint Coordinates;
	FMeshRenderData Ground;
	FMeshRenderData Water;

	void Clear()
	{
		Ground.Clear();
		Water.Clear();
	}
};
#pragma once

#include "CoreMinimal.h"
#include "MeshRenderData.h"


struct FSectorRenderData
{
	FIntPoint SectorCoordinates;
	
	FMeshRenderData GroundMeshRenderData;
	FMeshRenderData WaterMeshRenderData;

	void Clear()
	{
		GroundMeshRenderData.Clear();
		WaterMeshRenderData.Clear();
	}
};
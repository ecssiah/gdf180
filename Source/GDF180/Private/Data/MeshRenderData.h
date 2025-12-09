#pragma once

#include "CoreMinimal.h"


struct FMeshRenderData
{
	TArray<FVector3f> VertexArray;
	TArray<int32> IndexArray;
	TArray<FVector2f> UVArray;
	TArray<FVector4f> VertexColorArray;
	
	TArray<uint8> BoundaryMaskArray;
	
	TArray<uint8> PrimaryBiomeIndexArray;
	TArray<uint8> SecondaryBiomeIndexArray;

	void Clear()
	{
		VertexArray.Reset();
		IndexArray.Reset();
		UVArray.Reset();
		VertexColorArray.Reset();
		
		BoundaryMaskArray.Reset();
		
		PrimaryBiomeIndexArray.Reset();
		SecondaryBiomeIndexArray.Reset();
	}
};
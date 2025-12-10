#pragma once

#include "CoreMinimal.h"


struct FMeshRenderData
{
	TArray<FVector3f> VertexArray;
	TArray<int32> IndexArray;
	TArray<FVector2f> UVArray;
	TArray<uint8> BiomeIndexArray;
	TArray<FVector4f> VertexColorArray;

	void Clear()
	{
		VertexArray.Reset();
		IndexArray.Reset();
		UVArray.Reset();
		BiomeIndexArray.Reset();
		VertexColorArray.Reset();
	}
};
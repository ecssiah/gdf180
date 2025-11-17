#pragma once

#include "CoreMinimal.h"


struct FMeshRenderData
{
	TArray<FVector3f> VertexArray;
	TArray<int32> IndexArray;
	TArray<FVector2f> UVArray;
	TArray<FVector4f> VertexColorArray;

	void Clear()
	{
		VertexArray.Reset();
		IndexArray.Reset();
		UVArray.Reset();
		VertexColorArray.Reset();
	}
};
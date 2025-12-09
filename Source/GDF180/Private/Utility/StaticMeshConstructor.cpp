#include "StaticMeshConstructor.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshAttributes.h"
#include "MeshDescription.h"


TObjectPtr<UStaticMesh> FStaticMeshConstructor::Run(
    UObject* Outer,
    const TCHAR* MeshName,
    const FMeshRenderData& MeshRenderData,
    const bool bGenerateCollision
) {
    TObjectPtr<UStaticMesh> StaticMesh {
        NewObject<UStaticMesh>(
            Outer,
            MeshName,
            RF_Public | RF_Standalone
        )
    };

    FMeshDescription MeshDescription;
    FStaticMeshAttributes Attributes { MeshDescription };
    
    Attributes.Register();
    Attributes.GetVertexInstanceUVs().SetNumChannels(1);

    MeshDescription.ReserveNewVertices(MeshRenderData.VertexArray.Num());
    MeshDescription.ReserveNewVertexInstances(MeshRenderData.IndexArray.Num());
    MeshDescription.ReserveNewPolygons(MeshRenderData.IndexArray.Num() / 3);

    TMap<int32, FVertexID> VertexMap;
    VertexMap.Reserve(MeshRenderData.VertexArray.Num());

    for (int32 Index { 0 }; Index < MeshRenderData.VertexArray.Num(); Index++)
    {
        FVertexID VertexID { MeshDescription.CreateVertex() };
        
        Attributes.GetVertexPositions()[VertexID] = MeshRenderData.VertexArray[Index];

        VertexMap.Add(Index, VertexID);
    }
    
    const FPolygonGroupID PolygonGroup { MeshDescription.CreatePolygonGroup() };

    for (int32 Index { 0 }; Index < MeshRenderData.IndexArray.Num(); Index += 3)
    {
        const int32 Vertex0Index { MeshRenderData.IndexArray[Index + 0] }; 
        const int32 Vertex1Index { MeshRenderData.IndexArray[Index + 1] }; 
        const int32 Vertex2Index { MeshRenderData.IndexArray[Index + 2] }; 
        
        const FVertexInstanceID VertexInstanceID0 { MeshDescription.CreateVertexInstance(VertexMap[Vertex0Index]) };
        const FVertexInstanceID VertexInstanceID1 { MeshDescription.CreateVertexInstance(VertexMap[Vertex1Index]) };
        const FVertexInstanceID VertexInstanceID2 { MeshDescription.CreateVertexInstance(VertexMap[Vertex2Index]) };
        
        Attributes.GetVertexInstanceUVs().Set(
            VertexInstanceID0, 
            0, 
            MeshRenderData.UVArray[Vertex0Index]
        );
        
        Attributes.GetVertexInstanceUVs().Set(
            VertexInstanceID1, 
            0, 
            MeshRenderData.UVArray[Vertex1Index]
        );
        
        Attributes.GetVertexInstanceUVs().Set(
            VertexInstanceID2, 
            0, 
            MeshRenderData.UVArray[Vertex2Index]
        );
        
        Attributes.GetVertexInstanceColors().Set(
            VertexInstanceID0, 
            MeshRenderData.VertexColorArray[Vertex0Index]
        );
        
        Attributes.GetVertexInstanceColors().Set(
            VertexInstanceID1, 
            MeshRenderData.VertexColorArray[Vertex1Index]
        );
        
        Attributes.GetVertexInstanceColors().Set(
            VertexInstanceID2, 
            MeshRenderData.VertexColorArray[Vertex2Index]
        );

        TArray InstanceArray { VertexInstanceID0, VertexInstanceID1, VertexInstanceID2 };
        
        MeshDescription.CreatePolygon(PolygonGroup, InstanceArray);
    }

    StaticMesh->SetLightingGuid(FGuid::NewGuid());
    StaticMesh->SetNumSourceModels(1);

    if (StaticMesh->GetStaticMaterials().Num() == 0)
    {
        UMaterialInterface* DefaultMat { UMaterial::GetDefaultMaterial(MD_Surface) };
        StaticMesh->AddMaterial(DefaultMat);
    }
    
    FStaticMeshSourceModel& SourceModel { StaticMesh->GetSourceModel(0) };
    SourceModel.BuildSettings.bRecomputeNormals = true;
    SourceModel.BuildSettings.bRecomputeTangents = true;
    SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
    SourceModel.BuildSettings.bUseHighPrecisionTangentBasis = true;
    SourceModel.BuildSettings.bGenerateLightmapUVs = false;
    
    StaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
    StaticMesh->CommitMeshDescription(0);
    StaticMesh->ImportVersion = LastVersion;
    StaticMesh->Build(false);
    StaticMesh->PostEditChange();

    if (bGenerateCollision)
    {
        StaticMesh->bAllowCPUAccess = true;

        StaticMesh->CreateBodySetup();
        StaticMesh->GetBodySetup()->CollisionTraceFlag = CTF_UseComplexAsSimple;
        StaticMesh->GetBodySetup()->bMeshCollideAll = true;
        StaticMesh->GetBodySetup()->bHasCookedCollisionData = true;
        StaticMesh->GetBodySetup()->CreatePhysicsMeshes();
    }

    const bool _ { StaticMesh->MarkPackageDirty() };

    return StaticMesh;
}
#include "StaticMeshConstructor.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshAttributes.h"
#include "MeshDescription.h"


TObjectPtr<UStaticMesh> FStaticMeshConstructor::Run(
    UObject* Outer,
    const TCHAR* MeshName,
    const FMeshRenderData& RenderData,
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

    MeshDescription.ReserveNewVertices(RenderData.VertexArray.Num());
    MeshDescription.ReserveNewVertexInstances(RenderData.IndexArray.Num());
    MeshDescription.ReserveNewPolygons(RenderData.IndexArray.Num() / 3);

    TMap<int32, FVertexID> VertexMap;
    VertexMap.Reserve(RenderData.VertexArray.Num());

    for (int32 i { 0 }; i < RenderData.VertexArray.Num(); i++)
    {
        FVertexID VertexID { MeshDescription.CreateVertex() };
        
        Attributes.GetVertexPositions()[VertexID] = RenderData.VertexArray[i];

        VertexMap.Add(i, VertexID);
    }

    const FPolygonGroupID PolygonGroup { MeshDescription.CreatePolygonGroup() };

    for (int32 i { 0 }; i < RenderData.IndexArray.Num(); i += 3)
    {
        const FVertexInstanceID VertexInstanceID0 { 
            MeshDescription.CreateVertexInstance(VertexMap[RenderData.IndexArray[i + 0]]) 
        };
        
        const FVertexInstanceID VertexInstanceID1 { 
            MeshDescription.CreateVertexInstance(VertexMap[RenderData.IndexArray[i + 1]]) 
        };
        
        const FVertexInstanceID VertexInstanceID2 { 
            MeshDescription.CreateVertexInstance(VertexMap[RenderData.IndexArray[i + 2]]) 
        };

        Attributes.GetVertexInstanceUVs().Set(
            VertexInstanceID0, 
            0, 
            RenderData.UVArray[RenderData.IndexArray[i + 0]]
        );
        
        Attributes.GetVertexInstanceUVs().Set(
            VertexInstanceID1, 
            0, 
            RenderData.UVArray[RenderData.IndexArray[i + 1]]
        );
        
        Attributes.GetVertexInstanceUVs().Set(
            VertexInstanceID2, 
            0, 
            RenderData.UVArray[RenderData.IndexArray[i + 2]]
        );
        
        Attributes.GetVertexInstanceColors().Set(
            VertexInstanceID0, 
            RenderData.VertexColorArray[RenderData.IndexArray[i + 0]]
        );
        
        Attributes.GetVertexInstanceColors().Set(
            VertexInstanceID1, 
            RenderData.VertexColorArray[RenderData.IndexArray[i + 1]]
        );
        
        Attributes.GetVertexInstanceColors().Set(
            VertexInstanceID2, 
            RenderData.VertexColorArray[RenderData.IndexArray[i + 2]]
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

    const bool bSuppressed { StaticMesh->MarkPackageDirty() };

    UE_LOG(
        LogTemp,
        Log,
        TEXT("%s update suppressed: %s"),
        MeshName,
        bSuppressed ? TEXT("true") : TEXT("false")
    );
    
    return StaticMesh;
}
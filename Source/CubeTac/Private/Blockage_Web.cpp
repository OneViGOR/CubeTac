// Copyright 2019 James Vigor. All Rights Reserved.


#include "Blockage_Web.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

ABlockage_Web::ABlockage_Web() {

	//Set up blockage details
	bObstructAllMovement = false;	// Units can pass through webs
	AdditionalMovementCost = 1;		// Webs force units to spend 1 additional movement energy to leave the tile

	//Set up components
	//-Blockage Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BlockageMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/Web.Web'"));
	if (BlockageMeshAsset.Succeeded()) {
		BlockageMesh->SetStaticMesh(BlockageMeshAsset.Object);

	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlockageMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Blockages/WebDissolve.WebDissolve'"));
	if (BlockageMaterial.Succeeded()) {
		BlockageMesh->SetMaterial(0, BlockageMaterial.Object);
	}
}
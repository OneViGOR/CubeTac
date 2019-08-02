// Copyright 2019 James Vigor. All Rights Reserved.


#include "Blockage_TreeStump.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

ABlockage_TreeStump::ABlockage_TreeStump() {

	//Set up blockage details
	bObstructAllMovement = true;

	//Set up components
	//-Blockage Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BlockageMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/TreeStump.TreeStump'"));
	if (BlockageMeshAsset.Succeeded()) {
		BlockageMesh->SetStaticMesh(BlockageMeshAsset.Object);

	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlockageMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Wood.Wood'"));
	if (BlockageMaterial.Succeeded()) {
		BlockageMesh->SetMaterial(0, BlockageMaterial.Object);
	}
}
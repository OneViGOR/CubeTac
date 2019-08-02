// Copyright 2019 James Vigor. All Rights Reserved.


#include "BlockageC.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
ABlockageC::ABlockageC()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	bObstructAllMovement = true;

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Blockage Mesh
	BlockageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockageMesh"));
	BlockageMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BlockageMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cone.Cone'"));
	if (BlockageMeshAsset.Succeeded()) {
		BlockageMesh->SetStaticMesh(BlockageMeshAsset.Object);

	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlockageMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Red.Red'"));
	if (BlockageMaterial.Succeeded()) {
		BlockageMesh->SetMaterial(0, BlockageMaterial.Object);
	}
	BlockageMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ABlockageC::BeginPlay()
{
	Super::BeginPlay();
	
}
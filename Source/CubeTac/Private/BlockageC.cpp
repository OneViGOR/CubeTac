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

	//-Timeline
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));
	const ConstructorHelpers::FObjectFinder<UCurveFloat> DissolveCurve(TEXT("CurveFloat'/Game/GameData/Curves/1Sec_1to0.1Sec_1to0'"));
	if (DissolveCurve.Succeeded()) {
		FCurve = DissolveCurve.Object;
	}
	FOnTimelineFloat ProgressFunction{};
	ProgressFunction.BindUFunction(this, FName("DissolveTick")); // The function DissolveTick gets called when the timeline updates
	DissolveTimeline->AddInterpFloat(FCurve, ProgressFunction, FName("Alpha"));
}

// Called when the game starts or when spawned
void ABlockageC::BeginPlay()
{
	Super::BeginPlay();
	
	// Run the dissolve animation once.
	DissolveTimeline->SetLooping(false);
	DissolveTimeline->PlayFromStart();
}

// Called on a timeline to run the spawn dissolve animation.
void ABlockageC::DissolveTick(float Value)
{
	// Make a dynamic material from the mesh default material. If compatible with the animation, it will perform the dissolve. Otherwise it will remain static.
	UMaterialInstanceDynamic* DynMaterial = BlockageMesh->CreateDynamicMaterialInstance(0, BlockageMesh->GetMaterial(0));

	if (DynMaterial != nullptr) {
		DynMaterial->SetScalarParameterValue("Length", Value);
	}
}
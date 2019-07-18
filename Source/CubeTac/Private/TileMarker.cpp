// Fill out your copyright notice in the Description page of Project Settings.

#include "TileMarker.h"
#include "GridCharacterC.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
ATileMarker::ATileMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Cap Mesh
	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerMesh->SetupAttachment(RootComponent);
	MarkerMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MarkerMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (MarkerMeshAsset.Succeeded()) {
		MarkerMesh->SetStaticMesh(MarkerMeshAsset.Object);
	}

	//--Set Material References
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObjectMouseOver(TEXT("MaterialInstanceConstant'/Game/Materials/TileMarkers/TileHighlight_MouseOver.TileHighlight_MouseOver'"));
	if (MaterialObjectMouseOver.Succeeded()) {
		MaterialMouseOver = MaterialObjectMouseOver.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObjectSafe(TEXT("MaterialInstanceConstant'/Game/Materials/TileMarkers/TileHighlight_Safe.TileHighlight_Safe'"));
	if (MaterialObjectSafe.Succeeded()) {
		MaterialSafe = MaterialObjectSafe.Object;
	}																		
																			
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObjectDangerous(TEXT("MaterialInstanceConstant'/Game/Materials/TileMarkers/TileHighlight_Dangerous.TileHighlight_Dangerous'"));
	if (MaterialObjectDangerous.Succeeded()) {
		MaterialDangerous = MaterialObjectDangerous.Object;
	}																		
																			
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObjectTarget(TEXT("MaterialInstanceConstant'/Game/Materials/TileMarkers/TileHighlight_Target.TileHighlight_Target'"));
	if (MaterialObjectTarget.Succeeded()) {
		MaterialTarget = MaterialObjectTarget.Object;
	}																		
																			
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObjectSelected(TEXT("MaterialInstanceConstant'/Game/Materials/TileMarkers/TileHighlight_Selected.TileHighlight_Selected'"));
	if (MaterialObjectSelected.Succeeded()) {
		MaterialSelected = MaterialObjectSelected.Object;
	}																			
																				
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialObjectFullTransparent(TEXT("Material'/Game/Materials/PlainColours/FullTransparent.FullTransparent'"));
	if (MaterialObjectFullTransparent.Succeeded()) {
		MaterialFullTransparent = MaterialObjectFullTransparent.Object;
	}

	MarkerMesh->SetMaterial(0, MaterialMouseOver);

	//-Particle Systems
	ParticleMouseOver = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleMouseOver"));
	ParticleMouseOver->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObjectMouseOver(TEXT("ParticleSystem'/Game/VFX/TileEffects/TileMouseOver.TileMouseOver'"));
	if (ParticleObjectMouseOver.Succeeded()) {
		ParticleMouseOver->SetTemplate(ParticleObjectMouseOver.Object);
	}
	ParticleMouseOver->Deactivate();

	ParticleSafe = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSafe"));
	ParticleSafe->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObjectSafe(TEXT("ParticleSystem'/Game/VFX/TileEffects/TileSafe.TileSafe'"));
	if (ParticleObjectSafe.Succeeded()) {
		ParticleSafe->SetTemplate(ParticleObjectSafe.Object);
	}
	ParticleSafe->Deactivate();

	ParticleDangerous = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleDangerous"));
	ParticleDangerous->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObjectDangerous(TEXT("ParticleSystem'/Game/VFX/TileEffects/TileDangerous.TileDangerous'"));
	if (ParticleObjectDangerous.Succeeded()) {
		ParticleDangerous->SetTemplate(ParticleObjectDangerous.Object);
	}
	ParticleDangerous->Deactivate();

	ParticleTarget = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleTarget"));
	ParticleTarget->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObjectTarget(TEXT("ParticleSystem'/Game/VFX/TileEffects/TileTarget.TileTarget'"));
	if (ParticleObjectTarget.Succeeded()) {
		ParticleTarget->SetTemplate(ParticleObjectTarget.Object);
	}
	ParticleTarget->Deactivate();

	ParticleSelected = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSelected"));
	ParticleSelected->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObjectSelected(TEXT("ParticleSystem'/Game/VFX/TileEffects/TileSelected.TileSelected'"));
	if (ParticleObjectSelected.Succeeded()) {
		ParticleSelected->SetTemplate(ParticleObjectSelected.Object);
	}
	ParticleSelected->Deactivate();
}

void ATileMarker::BeginPlay() {
	Super::BeginPlay();
}

void ATileMarker::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void ATileMarker::UpdateAppearance(AActor_MapTile* tile)
{
	ParticleMouseOver->Deactivate();
	ParticleSafe->Deactivate();
	ParticleDangerous->Deactivate();
	ParticleTarget->Deactivate();
	ParticleSelected->Deactivate();

	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENavigationEnum"), true);
	FString EnumString = EnumPtr->GetNameStringByIndex((int32)tile->ECurrentlyNavigable);

	if (tile->bMouseOver) {
		MarkerMesh->SetMaterial(0, MaterialMouseOver);
		ParticleMouseOver->Activate();
	}
	else {
		switch (tile->ECurrentlyNavigable) {

		case ENavigationEnum::Nav_Safe:
			MarkerMesh->SetMaterial(0, MaterialSafe);
			ParticleSafe->Activate();
			break;
			
		case ENavigationEnum::Nav_Dangerous:
			MarkerMesh->SetMaterial(0, MaterialDangerous);
			ParticleDangerous->Activate();
			break;

		case ENavigationEnum::Nav_Unreachable:
			if (tile->bTargetable) {
				MarkerMesh->SetMaterial(0, MaterialTarget);
				ParticleTarget->Activate();
			}

			else if (tile->GetOccupyingCharacter()->IsValidLowLevel()){
				if (tile->GetOccupyingCharacter()->GetSelected()) {
					MarkerMesh->SetMaterial(0, MaterialSelected);
					ParticleSelected->Activate();
				}
				else {
					MarkerMesh->SetMaterial(0, MaterialFullTransparent);
				}
			}
			else {
				MarkerMesh->SetMaterial(0, MaterialFullTransparent);
			}
			break;
		}
	}
}
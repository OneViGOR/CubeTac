// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalC.h"
#include "TacticalGameState.h"
#include "GameMap.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

APortalC::APortalC(){

	//Set up components
	//-Character Mesh
	CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CharacterMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/Portal.Portal'"));
	if (CharacterMeshAsset.Succeeded()) {
		CharacterMesh->SetStaticMesh(CharacterMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Characters/PortalDissolve.PortalDissolve'"));
	if (CharacterMaterial.Succeeded()) {
		CharacterMesh->SetMaterial(0, CharacterMaterial.Object);
	}

	//-Rift Mesh
	RiftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiftPlane"));
	RiftMesh->SetupAttachment(RootComponent);
	RiftMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RiftMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/PortalPlane.PortalPlane'"));
	if (RiftMeshAsset.Succeeded()) {
		RiftMesh->SetStaticMesh(RiftMeshAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RiftMaterial(TEXT("Material'/Game/Materials/PortalRift.PortalRift'"));
	if (RiftMaterial.Succeeded()) {
		RiftMesh->SetMaterial(0, RiftMaterial.Object);
	}

}

void APortalC::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	if (world != nullptr) {
		FLocalPlayerContext Context = FLocalPlayerContext(world->GetFirstPlayerController());

		ATacticalGameState* GameState = Context.GetGameState<ATacticalGameState>();

		UMaterialInstanceDynamic* DynMaterial = RiftMesh->CreateDynamicMaterialInstance(0, RiftMesh->GetMaterial(0));

		if (DynMaterial != nullptr) {
			DynMaterial->SetVectorParameterValue("TeamColour", GameState->GetTeamColour(Team));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No DynMaterial"));
		}
	}

	for (TActorIterator<AGameMap> Itr(GetWorld()); Itr; ++Itr)
	{
		GameMapReference = *Itr;
	}

	FRotator FaceRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), GameMapReference->Midpoint->GetComponentLocation());
	SetActorRotation(FRotator(0.0f, FaceRot.Yaw, 0.0f));
}

void APortalC::SpawnCharacter(AMapTile* SpawnTile)
{

}
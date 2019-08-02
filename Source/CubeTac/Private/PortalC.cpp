// Copyright 2019 James Vigor. All Rights Reserved.


#include "PortalC.h"
#include "SpawnlingC.h"
#include "WebberC.h"
#include "UpholderC.h"
#include "TacticalGameState.h"
#include "PlayerPawnC.h"
#include "GameMap.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

APortalC::APortalC(){

	//Set up character details
	Name = FText::FromString("Portal");
	Health = 300;
	MaxHealth = 300;
	MovesRemaining = 0;
	TurnMoveDistance = 0;
	Energy = 40;
	MaxEnergy = 999;
	EnergyPerTurn = 10;
	SpawnRange = 2;

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
	RiftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//-Ambient Hum Sound
	AmbientHum = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioHum"));
	AmbientHum->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USoundBase> HumSoundAsset(TEXT("SoundWave'/Game/SFX/Ambience/PortalHum.PortalHum'"));
	if (HumSoundAsset.Succeeded()) {
		AmbientHum->SetSound(HumSoundAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> HumAttenuationAsset(TEXT("SoundAttenuation'/Game/SFX/Ambience/PortalAttenuation.PortalAttenuation'"));
	if (HumAttenuationAsset.Succeeded()) {
		AmbientHum->AttenuationSettings = HumAttenuationAsset.Object;
	}
	AmbientHum->bAutoActivate = true;

	//Spawn List
	SpawnList.Add(FCharacterBuyData(ASpawnlingC::StaticClass(), 25));
	SpawnList.Add(FCharacterBuyData(AWebberC::StaticClass(), 50));
	SpawnList.Add(FCharacterBuyData(AUpholderC::StaticClass(), 50));

}

void APortalC::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World != nullptr) {
		FLocalPlayerContext Context = FLocalPlayerContext(World->GetFirstPlayerController());

		ATacticalGameState* GameState = Context.GetGameState<ATacticalGameState>();

		UMaterialInstanceDynamic* DynMaterial = RiftMesh->CreateDynamicMaterialInstance(0, RiftMesh->GetMaterial(0));
		
		UE_LOG(LogTemp, Warning, TEXT("Team %d"), Team);
		
		if (DynMaterial != nullptr) {
			DynMaterial->SetVectorParameterValue("TeamColour", GameState->GetTeamColour(Team+1));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No DynMaterial"));
		}
	}

	for (TActorIterator<AGameMap> Itr(GetWorld()); Itr; ++Itr)
	{
		GameMapReference = *Itr;
	}

	FRotator FaceRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), FVector(0.0f, 0.0f, 0.0f));
	SetActorRotation(FRotator(0.0f, FaceRot.Yaw, 0.0f));
}

void APortalC::SpawnCharacter(AMapTile* SpawnTile)
{
	Cast<APlayerPawnC>(GetWorld()->GetFirstPlayerController()->GetPawn())->SpawnGridCharacter(SelectedSpawn.CharacterClass, SpawnTile, Team, this, SelectedSpawn.Price);
	CancelTargetting();
}

void APortalC::SelectSpawnCharacter(FCharacterBuyData SelectedData)
{
	CancelAllNavigableLocations();
	CancelTargetting();
	SelectedSpawn = SelectedData;
	TArray<AMapTile*> SpawnTiles = FindAbilityRange(SpawnRange);

	for (int i = 0; i < SpawnTiles.Num(); i++) {
		
		bool bTileIsTargetable = false;
		
		if (SpawnTiles[i] != nullptr) {
			if (SpawnTiles[i]->GetBlockage() != nullptr) {
				if (!SpawnTiles[i]->GetBlockage()->bObstructAllMovement) {
					bTileIsTargetable = true;
				}
			}
			else {
				bTileIsTargetable = true;
			}

			if (bTileIsTargetable) {
				if (SpawnTiles[i]->GetOccupyingCharacter() == nullptr) {
					SpawnTiles[i]->bTargetable = true;
					SpawnTiles[i]->SetHighlightMaterial();
				}
			}
			
		}
	}
}

void APortalC::NewTurnStart_Implementation()
{
	Energy += EnergyPerTurn;
}
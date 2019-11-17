// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Portal.h"
#include "Unit_Spawnling.h"
#include "Unit_Warden.h"
#include "Unit_Ranger.h"
#include "Unit_Genie.h"
#include "Unit_Webber.h"
#include "Unit_Upholder.h"
#include "TacticalGameState.h"
#include "PlayerPawnC.h"
#include "GameMap.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

AUnit_Portal::AUnit_Portal(){

	//Set up unit details
	Name = FText::FromString("Portal");
	IconAssetPath = "Texture2D'/Game/Textures/UI/UnitIcons/Portal.Portal'";
	Tier = 1;
	Health = 300;
	MaxHealth = 300;
	MovesRemaining = 0;
	TurnMoveDistance = 0;
	Energy = 40;
	MaxEnergy = 999;
	EnergyPerTurn = 10;
	SpawnRange = 2;
	MovementSpeed = 0.0f;

	//Unit Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> UnitIcon(*IconAssetPath);
	if (UnitIcon.Succeeded()) {
		Icon = UnitIcon.Object;
	}

	//Ability Icons
	static ConstructorHelpers::FObjectFinder<UTexture2D> UpgradeIcon(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'"));     // Icon for upgrade ability

	// Ability Data
	BaseAbilityData = FUnitAbility("Upgrade", UpgradeIcon.Object, 0.0f, 0, 2, false, false, false, false, false, false, false, false);      // Data structure for portal upgrade ability

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/Portal.Portal'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Characters/PortalDissolve.PortalDissolve'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitDeathMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Characters/PortalDeath.PortalDeath'"));
	if (UnitDeathMaterial.Succeeded()) {
		DeathDissolveMaterial = UnitDeathMaterial.Object;
	}

	//-Rift Mesh
	RiftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiftPlane"));
	RiftMesh->SetupAttachment(RootComponent);
	RiftMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RiftMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/PortalPlane.PortalPlane'"));
	if (RiftMeshAsset.Succeeded()) {
		RiftMesh->SetStaticMesh(RiftMeshAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RiftMaterial(TEXT("Material'/Game/Materials/Characters/PortalRipple.PortalRipple'"));
	if (RiftMaterial.Succeeded()) {
		RiftMesh->SetMaterial(0, RiftMaterial.Object);
	}
	RiftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	//-Team Colour Point Light
	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ColourPointLight"));
	PointLight->SetupAttachment(RootComponent);
	PointLight->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	PointLight->SetAttenuationRadius(200.0f);
	PointLight->SetIntensity(2000.0f);

	//-Ambient Hum Sound
	AmbientHum = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioHum"));
	AmbientHum->SetupAttachment(RootComponent);
	AmbientHum->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	static ConstructorHelpers::FObjectFinder<USoundBase> HumSoundAsset(TEXT("SoundWave'/Game/SFX/Ambience/PortalHum.PortalHum'"));
	if (HumSoundAsset.Succeeded()) {
		AmbientHum->SetSound(HumSoundAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> HumAttenuationAsset(TEXT("SoundAttenuation'/Game/SFX/Ambience/PortalAttenuation.PortalAttenuation'"));
	if (HumAttenuationAsset.Succeeded()) {
		AmbientHum->AttenuationSettings = HumAttenuationAsset.Object;
	}
	AmbientHum->bAutoActivate = true;

	//-Passive Particle Effect
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));
	ParticleSystem->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("ParticleSystem'/Game/VFX/PortalAmbience/Portal_Universal.Portal_Universal'"));
	ParticleSystem->SetTemplate(ParticleAsset.Object);

	//Spawn List
	//-Tier 1
	SpawnList.Add(FUnitBuyData(AUnit_Spawnling::StaticClass(), 25));
	SpawnList.Add(FUnitBuyData(AUnit_Warden::StaticClass(), 50));
	SpawnList.Add(FUnitBuyData(AUnit_Ranger::StaticClass(), 50));
	SpawnList.Add(FUnitBuyData(AUnit_Genie::StaticClass(), 50));
	SpawnList.Add(FUnitBuyData(AUnit_Webber::StaticClass(), 50));

	//-Tier 2
	SpawnList.Add(FUnitBuyData(AUnit_Upholder::StaticClass(), 50));
}

void AUnit_Portal::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World != nullptr) {
		FLocalPlayerContext Context = FLocalPlayerContext(World->GetFirstPlayerController());

		ATacticalGameState* GameState = Context.GetGameState<ATacticalGameState>();

		DynMaterial = RiftMesh->CreateDynamicMaterialInstance(0, RiftMesh->GetMaterial(0));
		
		// Set rift colour to the team colour found in the game state
		if (DynMaterial != nullptr) {
			DynMaterial->SetVectorParameterValue("TeamColour", GameState->GetTeamColour(Team));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No DynMaterial"));
		}
		FLinearColor TeamColour = GameState->GetTeamColour(Team);
		PointLight->SetLightColor(TeamColour);
		ParticleSystem->SetVectorParameter("Colour", FVector(TeamColour.R, TeamColour.G, TeamColour.B));
	}

	// Set reference to the game map to the game map actor found in the world (there should only be one)
	for (TActorIterator<AGameMap> Itr(GetWorld()); Itr; ++Itr)
	{
		GameMapReference = *Itr;
	}

	FRotator FaceRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), FVector(0.0f, 0.0f, 0.0f));
	SetActorRotation(FRotator(0.0f, FaceRot.Yaw, 0.0f));
}

// Portal death does same as standard units, but also checks whether the game should end as a result of a team being eliminated
void AUnit_Portal::Death() {
	Super::Death();

	FLocalPlayerContext Context = FLocalPlayerContext(GetWorld()->GetFirstPlayerController());
	ATacticalGameState* GameState = Context.GetGameState<ATacticalGameState>();
	Cast<ATacticalControllerBase>(GetInstigatorController())->SetPortal(nullptr); // Ensure reference to portal is cleared for owning player controller
	GameState->SetPortalDestroyedState(Team, true); // Record that this team's portal has been destroyed
	
}


// Spawn the selected unit type on a given tile
void AUnit_Portal::SpawnUnit(AMapTile* SpawnTile)
{
	// Spawn the unit
	Cast<APlayerPawnC>(GetWorld()->GetFirstPlayerController()->GetPawn())->SpawnGridUnit(SelectedSpawn.UnitClass, SpawnTile, Team, this, SelectedSpawn.Price);

	// Reset targeting and spawn selection
	CancelTargetting();
	SelectedSpawn = FUnitBuyData();
}

// Player selects which type of unit they wish to spawn
void AUnit_Portal::SelectSpawnUnit(FUnitBuyData SelectedData)
{
	// Cancel other active effects
	CancelAllNavigableLocations();
	CancelTargetting();

	SelectedSpawn = SelectedData;

	// Find all tiles within unit-spawning range
	TArray<AMapTile*> SpawnTiles = FindAbilityRange(SpawnRange);

	for (int i = 0; i < SpawnTiles.Num(); i++) {
		
		bool bTileIsTargetable = false;
		
		if (SpawnTiles[i] != nullptr) {
			// Ensure that the tile has no blockage that can obstruct unit movement
			if (SpawnTiles[i]->GetBlockage() != nullptr) {
				if (!SpawnTiles[i]->GetBlockage()->bObstructAllMovement) {
					bTileIsTargetable = true;
				}
			}
			else {
				bTileIsTargetable = true;
			}

			// As long as the above contitions are met, allow a unit to be spawned on this tile
			if (bTileIsTargetable) {
				if (SpawnTiles[i]->GetOccupyingUnit() == nullptr) {
					SpawnTiles[i]->bTargetable = true;
					SpawnTiles[i]->SetHighlightMaterial();
				}
			}
			
		}
	}
}

TArray<FUnitBuyData> AUnit_Portal::GetUnlockedSpawnList()
{
	TArray<FUnitBuyData> UnlockedSpawns;
	for (int i = 0; i < SpawnList.Num(); ++i) {
		AGridUnit* DefaultClass = SpawnList[i].UnitClass.GetDefaultObject();
		int FoundTier = DefaultClass->Tier;
		if (FoundTier <= Tier) {
			UnlockedSpawns.Add(SpawnList[i]);
		}
	}
	return UnlockedSpawns;
}

void AUnit_Portal::BaseAbility(AMapTile* TargetTile)
{
	Tier++;
	if (Tier == 2) {
		BaseAbilityData.Cost = 50;
	} else if (Tier == 3) {
		BaseAbilityData.Name = FText::FromString("Maxed Out");
	}
	SelectedSpawn = FUnitBuyData();
	SelectUnit();
}

// Deselects this unit - works as normal but also clears SelectedSpawn
void AUnit_Portal::DeselectUnit()
{
	SelectedSpawn = FUnitBuyData();
	Super::DeselectUnit();
}

// Adds energy to the portal's capacity at the start of its turn rather than resetting it to maximum value
void AUnit_Portal::NewTurnStart_Implementation()
{
	Energy += EnergyPerTurn;
}
// Copyright 2019 James Vigor. All Rights Reserved.


#include "PlayerPawnC.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"


// Sets default values
APlayerPawnC::APlayerPawnC()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->SetRelativeRotation(FRotator(330.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;

	//-Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	//Set up asset references
	//-Unit Spawn Sound
	static ConstructorHelpers::FObjectFinder<UParticleSystem> SpawnParticleAsset(TEXT("ParticleSystem'/Game/VFX/Spawn.Spawn'"));
	if (SpawnParticleAsset.Succeeded()) {
		SpawnParticles = SpawnParticleAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SpawnSoundAsset(TEXT("SoundWave'/Game/SFX/Action/Spawn.Spawn'"));
	if (SpawnSoundAsset.Succeeded()) {
		SpawnSound = SpawnSoundAsset.Object;
	}

	//-Unit Spawn Attenuation
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> SpawnAttenuationAsset(TEXT("SoundAttenuation'/Game/SFX/Action/SpawnAttenuation.SpawnAttenuation'"));
	if (SpawnAttenuationAsset.Succeeded()) {
		SpawnAttenuation = SpawnAttenuationAsset.Object;
	}
}

// Called to bind functionality to input
void APlayerPawnC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("CameraLook", IE_Pressed, this, &APlayerPawnC::InputCameraLookPressAction);
	PlayerInputComponent->BindAction("CameraLook", IE_Released, this, &APlayerPawnC::InputCameraLookReleaseAction);
	PlayerInputComponent->BindAxis("LookRight", this, &APlayerPawnC::InputLookRightAxis);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawnC::InputLookUpAxis);
	PlayerInputComponent->BindAxis("Zoom", this, &APlayerPawnC::InputZoomAxis);

}

// Deferred function called when camera look input is pressed, used for controlling units
void APlayerPawnC::InputCameraLookPressAction()
{
	bLooking = true;
}

// Deferred function called when camera look input is released, used for controlling units
void APlayerPawnC::InputCameraLookReleaseAction()
{
	bLooking = false;
}

// Deferred function using the look right axis value, used for controlling the camera
void APlayerPawnC::InputLookRightAxis(float Value)
{
	// Add a yaw angle scaled by the input value
	if (bLooking) {
		SpringArm->AddRelativeRotation(FRotator(0.0f, Value*2, 0.0f));
	}
}

// Deferred function using the look up axis value, used for controlling the camera
void APlayerPawnC::InputLookUpAxis(float Value)
{
	// Modify the world pitch angle of the spring arm by an amount scaled by the input value, and clamp the angle to sensible boundaries to keep the map in view
	if (bLooking) {
		SpringArm->SetWorldRotation(FRotator(FMath::ClampAngle(SpringArm->GetComponentRotation().Pitch + (Value * 2), 280.0f, 350.0f), SpringArm->GetComponentRotation().Yaw, 0.0f));
	}
}

// Deferred function using the zoom axis value, used for controlling the camera zoom
void APlayerPawnC::InputZoomAxis(float Value)
{
	// Add a modifier to the length of the spring arm, scaled by the input value, then clamp to sensible boundaries
	if (!bLobbyView) {
		SpringArm->TargetArmLength = (FMath::Clamp(SpringArm->TargetArmLength + (Value * -20), 700.0f, 1500.0f));
	}
}


//Brings the players out of the lobby view and begins the match
// - Validation
bool APlayerPawnC::BeginGame_Validate()
{
	return true;
}

// - Implementation
void APlayerPawnC::BeginGame_Implementation()
{
	bLobbyView = false;
	SpringArm->TargetArmLength = 1500;
	Cast<ATacticalControllerBase>(GetController())->SetUpGameUI();
}


// Moves a selected unit to a new tile
// - Validation
bool APlayerPawnC::MoveUnit_Validate(AMapTile* MoveToTile, ATacticalControllerBase* PlayerController)
{
	return true;
}

// - Implementation
void APlayerPawnC::MoveUnit_Implementation(AMapTile* MoveToTile, ATacticalControllerBase* PlayerController)
{
	AGridUnit* MovingUnit = PlayerController->SelectedUnit;
	AMapTile* CurrentTile = MovingUnit->GetCurrentTile();

	// Rotate to face direction of movement
	FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(CurrentTile->GetActorLocation(), MoveToTile->GetActorLocation());
	MovingUnit->SetActorRotation(FRotator(0.0f, LookRotation.Yaw, 0.0f));

	// Assign values and calculate aftermath
	CurrentTile->SetOccupyingUnit(nullptr);
	MovingUnit->SetActorLocation(MoveToTile->GetActorLocation());
	MovingUnit->DetermineCurrentTile();
	ClientMovement(MovingUnit, MoveToTile, CurrentTile);
}


// Determines the aftermath of the movement. Spends movement points and inflicts falling damage
// - Validation
bool APlayerPawnC::ClientMovement_Validate(AGridUnit* Unit, AMapTile * DestinationTile, AMapTile * CurrentTile)
{
	return true;
}

// Implementation
void APlayerPawnC::ClientMovement_Implementation(AGridUnit* Unit, AMapTile * DestinationTile, AMapTile * CurrentTile)
{
	bool bUnitDead = false;

	if (DestinationTile->ECurrentlyNavigable == ENavigationEnum::Nav_Dangerous) {
		// Inflict falling damage and clear all remaining movement points
		bUnitDead = DetermineMovementDamage(Unit, DestinationTile, CurrentTile);
		Unit->SetMovesRemaining(0);
	}
	else {
		int MovesSpent;
		if (Unit->DoesIgnoreBlockageSlowing()) {
			// Find movement cost excluding blockage
			MovesSpent = CurrentTile->GetBaseMovementCost();
		}
		else {
			// Find movement cost including blockage
			MovesSpent = CurrentTile->GetTotalMovementCost();
		}

		Unit->SpendMoves(MovesSpent);
	}

	// If unit is still alive, show the next possible set of locations it may move to
	if (!bUnitDead) {
		Unit->ShowNavigableLocations(DestinationTile);
	}
}


// Destroys an actor, ensuring that all clients experience it
// - Validation
bool APlayerPawnC::DestroyActor_Validate(AActor* Target)
{
	return Target != nullptr;
}

// - Implementation
void APlayerPawnC::DestroyActor_Implementation(AActor* Target)
{
	Target->Destroy();
}


// Create a new portal unit
// - Validation
bool APlayerPawnC::SpawnPortal_Validate(AMapTile* Tile, int Team)
{
	return true;
}

// - Implementation
void APlayerPawnC::SpawnPortal_Implementation(AMapTile* Tile, int Team)
{
	FTransform Transform = FTransform(Tile->GetActorRotation(), Tile->GetActorLocation(), FVector(1, 1, 1));

	// Ensure the portal's team is set before spawning finishes
	AUnit_Portal* NewPortal = GetWorld()->SpawnActorDeferred<AUnit_Portal>(AUnit_Portal::StaticClass(), Transform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewPortal) {
		NewPortal->SetTeam(Team);
		UGameplayStatics::FinishSpawningActor(NewPortal, Transform);
	}
	
	// Assign portal to the player controller and to the tile it spawned on
	Cast<ATacticalControllerBase>(GetController())->SetPortal(NewPortal);
	Tile->SetOccupyingUnit(NewPortal);

	// Play particle effect and audio
	SpawnEffects(SpawnParticles, Transform.GetLocation());
}


// Create a new unit
// - Validation
bool APlayerPawnC::SpawnGridUnit_Validate(TSubclassOf<class AGridUnit> UnitClass, AMapTile* Tile, int Team, AUnit_Portal* Portal, int Cost)
{
	return true;
}

// - Implementation
void APlayerPawnC::SpawnGridUnit_Implementation(TSubclassOf<class AGridUnit> UnitClass, AMapTile* Tile, int Team, AUnit_Portal* Portal, int Cost)
{
	FTransform Transform = FTransform(Tile->GetActorRotation(), Tile->GetActorLocation(), FVector(1, 1, 1));

	// Ensure the unit's team is set before spawning finishes
	AGridUnit* NewUnit = GetWorld()->SpawnActorDeferred<AGridUnit>(UnitClass, Transform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewUnit) {
		NewUnit->SetTeam(Team);
		UGameplayStatics::FinishSpawningActor(NewUnit, Transform);
	}

	Tile->SetOccupyingUnit(NewUnit);  // Assign unit to the tile it spawned on

	NewUnit->SetOwner(this);

	// Spend required energy
	if (Portal != nullptr) {
		Portal->SpendEnergy(Cost);
	}

	// Play particle effect and audio
	SpawnEffects(SpawnParticles, Transform.GetLocation());
}


// Calls for the effects that play when a unit spawns
// - Validation
bool APlayerPawnC::SpawnEffects_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

// - Implementation
void APlayerPawnC::SpawnEffects_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplate, Location, FRotator::ZeroRotator, true);
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SpawnSound, Location, FRotator::ZeroRotator, 1, 1, 0, SpawnAttenuation, nullptr, true);
}


// Calculate and apply damage to a unit based on the distance it fell
bool APlayerPawnC::DetermineMovementDamage(AGridUnit * Unit, AMapTile * FromTile, AMapTile * ToTile)
{
	//The difference in height between the destination and the fall damage threshold
	float HeightDifference = FMath::Abs((FromTile->GetActorLocation().Z - Unit->GetMoveClimbHeight()) - ToTile->GetActorLocation().Z);

	//Fall damage is HeightDifference / 5
	int Damage = FMath::CeilToInt(HeightDifference / 5);

	//Determine whether the damage is enough to kill the falling unit
	bool bUnitDead = Unit->GetHealth() <= Damage;

	//Apply damage to unit and report whether they were killed by the fall
	Unit->DamageTarget(Unit, Damage);
	return bUnitDead;
}
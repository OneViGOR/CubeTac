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
	//-Character Spawn Sound
	static ConstructorHelpers::FObjectFinder<UParticleSystem> SpawnParticleAsset(TEXT("ParticleSystem'/Game/VFX/Spawn.Spawn'"));
	if (SpawnParticleAsset.Succeeded()) {
		SpawnParticles = SpawnParticleAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SpawnSoundAsset(TEXT("SoundWave'/Game/SFX/Action/Spawn.Spawn'"));
	if (SpawnSoundAsset.Succeeded()) {
		SpawnSound = SpawnSoundAsset.Object;
	}

	//-Character Spawn Attenuation
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

// Input functions
void APlayerPawnC::InputCameraLookPressAction()
{
	bLooking = true;
}

void APlayerPawnC::InputCameraLookReleaseAction()
{
	bLooking = false;
}

void APlayerPawnC::InputLookRightAxis(float Value)
{
	if (bLooking) {
		SpringArm->AddRelativeRotation(FRotator(0.0f, Value*2, 0.0f));
	}
}

void APlayerPawnC::InputLookUpAxis(float Value)
{
	if (bLooking) {
		SpringArm->SetWorldRotation(FRotator(FMath::ClampAngle(SpringArm->GetComponentRotation().Pitch + (Value * 2), 280.0f, 350.0f), SpringArm->GetComponentRotation().Yaw, 0.0f));
	}
}

void APlayerPawnC::InputZoomAxis(float Value)
{
	if (!bLobbyView) {
		SpringArm->TargetArmLength = (FMath::Clamp(SpringArm->TargetArmLength + (Value * -20), 700.0f, 1500.0f));
	}
}


bool APlayerPawnC::BeginGame_Validate()
{
	return true;
}

void APlayerPawnC::BeginGame_Implementation()
{
	bLobbyView = false;
	SpringArm->TargetArmLength = 1500;
	Cast<ATacticalControllerBase>(GetController())->SetUpGameUI();
}


bool APlayerPawnC::MoveCharacter_Validate(AMapTile* MoveToTile, ATacticalControllerBase* CharacterController)
{
	return true;
}

void APlayerPawnC::MoveCharacter_Implementation(AMapTile* MoveToTile, ATacticalControllerBase* CharacterController)
{
	AGridCharacterC* MovingCharacter = CharacterController->SelectedCharacter;
	AMapTile* CurrentTile = MovingCharacter->GetCurrentTile();

	MovingCharacter->GetCurrentTile()->SetOccupyingCharacter(nullptr);
	MovingCharacter->SetActorLocation(MoveToTile->GetActorLocation());

	FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(CurrentTile->GetActorLocation(), MoveToTile->GetActorLocation());
	MovingCharacter->SetActorRotation(FRotator(0.0f, LookRotation.Yaw, 0.0f));
	MovingCharacter->DetermineCurrentTile();
	ClientMovement(MovingCharacter, MoveToTile, CurrentTile);
}


bool APlayerPawnC::ClientMovement_Validate(AGridCharacterC * Character, AMapTile * DestinationTile, AMapTile * CurrentTile)
{
	return true;
}

void APlayerPawnC::ClientMovement_Implementation(AGridCharacterC * Character, AMapTile * DestinationTile, AMapTile * CurrentTile)
{
	bool bCharacterDead = false;

	if (DestinationTile->ECurrentlyNavigable == ENavigationEnum::Nav_Dangerous) {
		//bCharacterDead = DETERMINE MOVEMENT DAMAGE
		Character->SetMovesRemaining(0);
	}
	else {
		int MovesSpent;
		if (Character->DoesIgnoreBlockageSlowing()) {
			MovesSpent = CurrentTile->GetTotalMovementCost();
		}
		else {
			MovesSpent = 1;
		}
		Character->SpendMoves(MovesSpent);
	}

	if (!bCharacterDead) {
		Character->ShowNavigableLocations(DestinationTile);
	}
}


bool APlayerPawnC::DestroyActor_Validate(AActor* Target)
{
	return Target->IsValidLowLevel();
}

void APlayerPawnC::DestroyActor_Implementation(AActor* Target)
{
	Target->Destroy();
}


bool APlayerPawnC::SpawnPortal_Validate(AMapTile* Tile, int Team)
{
	return true;
}

void APlayerPawnC::SpawnPortal_Implementation(AMapTile* Tile, int Team)
{
	FTransform Transform = FTransform(Tile->GetActorRotation(), Tile->GetActorLocation(), FVector(1, 1, 1));

	APortalC* NewPortal = GetWorld()->SpawnActorDeferred<APortalC>(APortalC::StaticClass(), Transform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewPortal) {
		NewPortal->SetTeam(Team);
		UGameplayStatics::FinishSpawningActor(NewPortal, Transform);
	}
	
	Cast<ATacticalControllerBase>(GetController())->OwnedPortal = NewPortal;
	Tile->SetOccupyingCharacter(NewPortal);
}


bool APlayerPawnC::SpawnGridCharacter_Validate(TSubclassOf<class AGridCharacterC> CharacterClass, AMapTile* Tile, int Team, APortalC* Portal, int Cost)
{
	return true;
}

void APlayerPawnC::SpawnGridCharacter_Implementation(TSubclassOf<class AGridCharacterC> CharacterClass, AMapTile* Tile, int Team, APortalC* Portal, int Cost)
{
	FTransform Transform = FTransform(Tile->GetActorRotation(), Tile->GetActorLocation(), FVector(1, 1, 1));

	AGridCharacterC* NewCharacter = GetWorld()->SpawnActorDeferred<AGridCharacterC>(CharacterClass, Transform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewCharacter) {
		NewCharacter->SetTeam(Team);
		UGameplayStatics::FinishSpawningActor(NewCharacter, Transform);
	}

	Tile->SetOccupyingCharacter(NewCharacter);

	NewCharacter->SetOwner(this);

	if (Portal != nullptr) {
		Portal->SpendEnergy(Cost);
	}

	SpawnEffects(SpawnParticles, Transform.GetLocation());
}


bool APlayerPawnC::SpawnEffects_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

void APlayerPawnC::SpawnEffects_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	UE_LOG(LogTemp, Warning, TEXT("SpawnEffects"));
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplate, Location, FRotator::ZeroRotator, true);
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SpawnSound, Location, FRotator::ZeroRotator, 1, 1, 0, SpawnAttenuation, nullptr, true);
}
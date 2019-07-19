// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnC.h"


// Sets default values
APlayerPawnC::APlayerPawnC()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->SetRelativeRotation(FRotator(0.0f, 330.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;

	//-Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);


}

// Called to bind functionality to input
void APlayerPawnC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("CameraLook", IE_Pressed, this, &APlayerPawnC::InputCameraLookPressAction);
	PlayerInputComponent->BindAction("CameraLook", IE_Released, this, &APlayerPawnC::InputCameraLookReleaseAction);
	PlayerInputComponent->BindAxis("LookRight", this, &APlayerPawnC::InputLookRightAxis);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawnC::InputLookUpAxis);

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

void APlayerPawnC::MoveCharacter(AMapTile* MoveToTile, ATacticalControllerBase* CharacterController)
{

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
	UE_LOG(LogTemp, Warning, TEXT("Spawning"));
	FVector Location = Tile->GetActorLocation();
	FRotator Rotation = Tile->GetActorRotation();
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APortalC* NewPortal = GetWorld()->SpawnActor<APortalC>(Location, Rotation, SpawnParameters);
	Cast<ATacticalControllerBase>(GetController())->OwnedPortal = NewPortal;
	Tile->SetOccupyingCharacter(NewPortal);
	
}
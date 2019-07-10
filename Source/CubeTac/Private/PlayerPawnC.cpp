// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnC.h"

// Sets default values
APlayerPawnC::APlayerPawnC()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerPawnC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerPawnC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerPawnC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerPawnC::MoveCharacter(AActor_MapTile* MoveToTile, ATacticalControllerC* CharacterController)
{

}

void APlayerPawnC::DestroyActor(AActor* Target)
{

}

void APlayerPawnC::SpawnPortal(AActor_MapTile* Tile, int Team)
{

}
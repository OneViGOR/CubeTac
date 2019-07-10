// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor_MapTile.h"
#include "TacticalControllerC.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawnC.generated.h"

UCLASS()
class CUBETAC_API APlayerPawnC : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawnC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveCharacter(AActor_MapTile* MoveToTile, ATacticalControllerC* CharacterController);
	void DestroyActor(AActor* Target);
	void SpawnPortal(AActor_MapTile* Tile, int Team);

};

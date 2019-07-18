// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor_MapTile.h"
#include "TacticalControllerC.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
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
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;

	UPROPERTY()
		USpringArmComponent* SpringArm;
	UPROPERTY()
		UCameraComponent* Camera;

	//Variables
	bool bLobbyView;
	bool bLooking;

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Input functions
	void InputCameraLookPressAction();
	void InputCameraLookReleaseAction();
	void InputLookRightAxis(float Value);
	void InputLookUpAxis(float Value);

	void MoveCharacter(AActor_MapTile* MoveToTile, ATacticalControllerC* CharacterController);
	void DestroyActor(AActor* Target);
	void SpawnPortal(AActor_MapTile* Tile, int Team);

};

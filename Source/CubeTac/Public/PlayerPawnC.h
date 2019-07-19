// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapTile.h"
#include "TacticalControllerBase.h"
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
	UFUNCTION()
		void InputCameraLookPressAction();
	UFUNCTION()
		void InputCameraLookReleaseAction();
	UFUNCTION()
		void InputLookRightAxis(float Value);
	UFUNCTION()
		void InputLookUpAxis(float Value);

	//Game Management
	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
		void BeginGame();

	UFUNCTION()
		void MoveCharacter(AMapTile* MoveToTile, ATacticalControllerBase* CharacterController);
	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyActor(AActor* Target);
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnPortal(AMapTile* Tile, int Team);

};

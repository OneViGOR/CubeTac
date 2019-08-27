// Copyright 2019 James Vigor. All Rights Reserved.

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
	/**
	*   COMPONENTS
	*/

	// Root component provides transform data
	UPROPERTY()
		USceneComponent* SceneRoot;

	// Spring arm for the camera so that it pivots around the centre of the game map
	UPROPERTY()
		USpringArmComponent* SpringArm;

	// Camera provides the player view
	UPROPERTY()
		UCameraComponent* Camera;


	/**
	*   VARIABLES
	*/

	// True if the game is in the lobby phase, so the player has no direct game control. Otherwise, false
	bool bLobbyView = true;

	// True if the player is controlling the camera. Otherwise, false so the player can control their units+
	bool bLooking;

	// Pointer to particle system asset for spawning units
	UParticleSystem* SpawnParticles;

	// Pointer to sound wave asset for spawning units
	USoundBase* SpawnSound;

	// Pointer to audio attenuation asset to spawning units
	USoundAttenuation* SpawnAttenuation;

public:
	/**
	*   FUNCTIONS
	*/

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Deferred function called when camera look input is pressed, used for controlling units
	UFUNCTION()
		void InputCameraLookPressAction();

	// Deferred function called when camera look input is released, used for controlling units
	UFUNCTION()
		void InputCameraLookReleaseAction();

	// Deferred function using the look right axis value, used for controlling the camera
	UFUNCTION()
		void InputLookRightAxis(float Value);

	// Deferred function using the look up axis value, used for controlling the camera
	UFUNCTION()
		void InputLookUpAxis(float Value);

	// Deferred function using the zoom axis value, used for controlling the camera zoom
	UFUNCTION()
		void InputZoomAxis(float Value);

	//Brings the players out of the lobby view and begins the match
	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
		void BeginGame();

	/**
	*   Moves a selected unit to a new tile
	*
	*	@Param		MoveToTile				Tile to move the unit to
	*	@Param		PlayerController		Controller owning the unit to be moved
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void MoveUnit(AMapTile* MoveToTile, ATacticalControllerBase* PlayerController);

	/**
	*   Determines the aftermath of the movement. Spends movement points and inflicts falling damage
	*
	*	@Param		Unit				Unit that moved
	*	@Param		DestinationTile		Tile the unit moved to
	*/
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientMovement(AGridUnit* Unit, AMapTile* DestinationTile, AMapTile* CurrentTile);

	/**
	*   Destroys an actor, ensuring that all clients experience it
	*
	*	@Param		Target				The actor to be destroyed
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyActor(AActor* Target);

	/**
	*   Create a new portal unit
	*
	*	@Param		Tile			Tile for the new portal to occupy
	*	@Param		Team			Index of the team the portal should belong to (as an integer 1-4)
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnPortal(AMapTile* Tile, int Team);

	/**
	*   Create a new unit
	*
	*	@Param		UnitClass			Type of unit to spawn as a class reference
	*	@Param		Tile				Tile for the new portal to occupy
	*	@Param		Team				Index of the team the portal should belong to (as an integer 1-4)
	*	@Param		Portal				Portal responsible for spawning the new unit
	*	@Param		Cost				Amount of energy that must be deducted from the portal to spawn the new unit
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnGridUnit(TSubclassOf<class AGridUnit> UnitClass, AMapTile* Tile, int Team, AUnit_Portal* Portal, int Cost);

	/**
	*   Calls for the effects that play when a unit spawns
	*
	*	@Param		EmitterTemplate			Type of particle system to spawn
	*	@Param		Location				Position in world space to spawn the emitter at
	*/
	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void SpawnEffects(UParticleSystem* EmitterTemplate, FVector Location);

	/**
	*   Calculate and apply damage to a unit based on the distance it fell
	*
	*	@Param		Unit			Unit that fell
	*	@Param		FromTile		Tile the unit fell from
	*	@Param		ToTile			Tile the unit fell to
	*/
	UFUNCTION()
		bool DetermineMovementDamage(AGridUnit* Unit, AMapTile* FromTile, AMapTile* ToTile);
};

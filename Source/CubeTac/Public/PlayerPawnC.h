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
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;

	UPROPERTY()
		USpringArmComponent* SpringArm;
	UPROPERTY()
		UCameraComponent* Camera;

	//Variables
	bool bLobbyView = true;
	bool bLooking;

	//Asset References
	UParticleSystem* SpawnParticles;
	USoundBase* SpawnSound;
	USoundAttenuation* SpawnAttenuation;

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
	UFUNCTION()
		void InputZoomAxis(float Value);

	//Game Management
	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
		void BeginGame();

	//Character Functions
	UFUNCTION(Server, Reliable, WithValidation)
		void MoveCharacter(AMapTile* MoveToTile, ATacticalControllerBase* CharacterController);
	UFUNCTION(Client, Reliable, WithValidation)
		void ClientMovement(AGridCharacterC* Character, AMapTile* DestinationTile, AMapTile* CurrentTile);
	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyActor(AActor* Target);
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnPortal(AMapTile* Tile, int Team);
	UFUNCTION(Server, Reliable, WithValidation)
		void SpawnGridCharacter(TSubclassOf<class AGridCharacterC> CharacterClass, AMapTile* Tile, int Team, APortalC* Portal, int Cost);
	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void SpawnEffects(UParticleSystem* EmitterTemplate, FVector Location);
};

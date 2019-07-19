// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TacticalControllerBase.h"
#include "TacticalGameState.h"
#include "GameFramework/GameModeBase.h"
#include "CubeTacGameModeBase.generated.h"


UCLASS()
class CUBETAC_API ACubeTacGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ACubeTacGameModeBase();

protected:
	int TeamPlaying = 1;
	int NumberOfPlayers;
	TArray<ATacticalControllerBase*> PlayerControllers;
	ATacticalGameState* GameStateReference;

public:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation)
		void PassTurn(ATacticalControllerBase* PlayerController);
	UFUNCTION()
		void ResetPlayerPieces();
};

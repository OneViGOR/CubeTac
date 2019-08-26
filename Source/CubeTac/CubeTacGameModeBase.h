// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TacticalGameState.h"
#include "GameFramework/GameModeBase.h"
#include "CubeTacGameModeBase.generated.h"

class ATacticalControllerBase;

// An enumeration to define whether a player wins, loses or draws a game
UENUM()
enum class EVictoryState : uint8 {
	State_Victorious	UMETA(DisplayName = "Victorious"),
	State_Draw			UMETA(DisplayName = "Draw"),
	State_Defeated		UMETA(DisplayName = "Defeated")
};

UCLASS()
class CUBETAC_API ACubeTacGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ACubeTacGameModeBase();

protected:
	/**
	*	VARIABLES
	*/

	// Current active team as an integer 1-4
	int TeamPlaying = 1;

	// The number of players currently in the game
	int NumberOfPlayers;

	// An array of references to all player controllers connected to the game
	TArray<ATacticalControllerBase*> PlayerControllers;

	// A reference to the active game state
	ATacticalGameState* GameStateReference;

public:
	/**
	*	FUNCTIONS
	*/

	virtual void BeginPlay() override;

	// New player joins the game, is given a pawn and a team
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Game turn is passed to the next player
	UFUNCTION(Server, Reliable, WithValidation)
		void PassTurn(ATacticalControllerBase* PlayerController);

	// Refresh all of the current team's units for the start of their new turn
	UFUNCTION()
		void ResetPlayerPieces();

	// Check whether the game should continue after a potentially game-ending event
	UFUNCTION(Server, Reliable, WithValidation)
		void CheckGameEnded();
};

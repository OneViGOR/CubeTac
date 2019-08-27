// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TacticalGameState.generated.h"

// An enumeration to define the distinct phases of game progression
UENUM(BlueprintType)
enum class EGamePhase : uint8 {
	Phase_Lobby			UMETA(DisplayName = "Lobby"),
	Phase_Portal		UMETA(DisplayName = "Portal Placement"),
	Phase_Game			UMETA(DisplayName = "Gameplay"),
	Phase_End			UMETA(DisplayName = "End of Game")
};

UCLASS()
class CUBETAC_API ATacticalGameState : public AGameStateBase
{
	GENERATED_BODY()
	

protected:
	/**
	*   VARIABLES
	*/

	// A reference to the four team colours. These can be referenced from various places to ensure uniformity in the teams' colour schemes
	UPROPERTY(BlueprintReadOnly)
		TArray<FLinearColor> TeamColours = { FLinearColor(0.0f, 0.309435f, 1.0f, 1.0f), FLinearColor(1.0f, 0.013363f, 0.0f, 1.0f), FLinearColor(0.073495f, 0.696875f, 0.0f, 1.0f), FLinearColor(1.0f, 0.425576f, 0.0f, 1.0f) };
	
	// The current phase of the game as an anumeration
	UPROPERTY(Replicated)
		EGamePhase GamePhase;

	// The team currently taking their turn, as an integer 1-4
	UPROPERTY(Replicated, BlueprintReadOnly)
		int TeamPlaying = 1;

	// The number of players currently connected to this game
	UPROPERTY(Replicated, BlueprintReadOnly)
		int NumberOfPlayers;

	// Each element refers to the state of a team's portal (index = team number - 1) where true means the portal has been destroyed
	UPROPERTY(Replicated, BlueprintReadOnly)
		TArray<bool> TeamPortalsDestroyed;

public:
	/**
	*   FUNCTIONS
	*/

	// Executes once every frame. Unused.
	virtual void Tick(float DeltaTime) override;

	// Returns the enumeration of the current game phase
	UFUNCTION(BlueprintCallable)
		EGamePhase GetGamePhase();

	/**
	*	Set the current game phase
	*
	*	@Param		NewGamePhase		The game phase to transition to
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void SetGamePhase(EGamePhase NewGamePhase);

	/**
	*	Move the game along so that a new team can take their turn
	*
	*	@Param		TeamNumber		The team number of the team to take their turn, as an integer 1-4
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void SetTeamPlaying(int TeamNumber);

	/**
	*	Set the number of players in the game
	*
	*	@Param		NewNumber		The new number of players in the game
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void SetNumberOfPlayers(int NewNumber);

	/**
	*	Set whether a portal has been destroyed or not
	*
	*	@Param		Team				The team number of the portal to reference, as an integer 1-4
	*	@Param		PortalDestroyed		True if the portal is destroyed, otherwise false
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void SetPortalDestroyedState(int Team, bool PortalDestroyed);

	// Returns the array of portal states
	UFUNCTION()
		TArray<bool> GetPortalDestroyedStateArray();

	/**
	*	Returns the colour to be used by the given team
	*
	*	@Param		TeamNumber		The team number to get the colour for, as an integer 1-4
	*/
	UFUNCTION()
		FLinearColor GetTeamColour(int TeamNumber);

};

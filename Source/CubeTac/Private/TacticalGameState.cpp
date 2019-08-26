// Copyright 2019 James Vigor. All Rights Reserved.


#include "TacticalGameState.h"
#include "CubeTacGameModeBase.h"
#include "Engine.h"
#include "UnrealNetwork.h"


// Executes once every frame. Unused.
void ATacticalGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATacticalGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(ATacticalGameState, GamePhase);
	DOREPLIFETIME(ATacticalGameState, TeamPlaying);
	DOREPLIFETIME(ATacticalGameState, NumberOfPlayers);
	DOREPLIFETIME(ATacticalGameState, TeamPortalsDestroyed);
}

// Returns the enumeration of the current game phase
EGamePhase ATacticalGameState::GetGamePhase()
{
	return GamePhase;
}


// Set the current game phase
// - Validation
bool ATacticalGameState::SetGamePhase_Validate(EGamePhase NewGamePhase)
{
	return true;
}

// - Implementation
void ATacticalGameState::SetGamePhase_Implementation(EGamePhase NewGamePhase)
{
	GamePhase = NewGamePhase;
}


// Move the game along so that a new team can take their turn
// - Validation
bool ATacticalGameState::SetTeamPlaying_Validate(int TeamNumber)
{
	return true;
}

// - Implementation
void ATacticalGameState::SetTeamPlaying_Implementation(int TeamNumber)
{
	TeamPlaying = TeamNumber;
}


// Set the number of players in the game
// - Validation
bool ATacticalGameState::SetNumberOfPlayers_Validate(int NewNumber)
{
	return true;
}

// - Implementation
void ATacticalGameState::SetNumberOfPlayers_Implementation(int NewNumber)
{
	NumberOfPlayers = NewNumber;
}


// Set whether a portal has been destroyed or not
// - Validation
bool ATacticalGameState::SetPortalDestroyedState_Validate(int Team, bool PortalDestroyed)
{
	return true;
}

// - Implementation
void ATacticalGameState::SetPortalDestroyedState_Implementation(int Team, bool PortalDestroyed)
{
	if (TeamPortalsDestroyed.Num() == Team - 1) { //Must add new team record. This should happen when a new player joins the game session.
		TeamPortalsDestroyed.Add(PortalDestroyed);
	}
	else if (TeamPortalsDestroyed.Num() >= Team) { //Team record already exists - just set new value. This should happen when a player's portal is destroyed.
		TeamPortalsDestroyed[Team - 1] = PortalDestroyed;
	}
	else { //Function is attempting to add a record for a team at least two numbers higher than the highest numbered team. Teams must be added sequentially. This should never happen.
		UE_LOG(LogTemp, Error, TEXT("Attempted to add a team out of sequence. Tried to add team %d before team %d exists."), Team, Team - 1);
	}

	Cast<ACubeTacGameModeBase>(GetWorld()->GetAuthGameMode())->CheckGameEnded();
}


// Returns the array of portal states
TArray<bool> ATacticalGameState::GetPortalDestroyedStateArray() {
	return TeamPortalsDestroyed;
}

// Returns the colour to be used by the given team
FLinearColor ATacticalGameState::GetTeamColour(int TeamNumber)
{
	if (TeamNumber > 0) {
		return TeamColours[FMath::Clamp(TeamNumber - 1, 0, 3)];
	}
	else {
		return FLinearColor(0,0,0,1);
	}
}
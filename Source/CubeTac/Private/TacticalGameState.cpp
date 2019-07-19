// Fill out your copyright notice in the Description page of Project Settings.


#include "TacticalGameState.h"
#include "UnrealNetwork.h"

void ATacticalGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(ATacticalGameState, GamePhase);
	DOREPLIFETIME(ATacticalGameState, TeamPlaying);
	DOREPLIFETIME(ATacticalGameState, NumberOfPlayers);

}

EGamePhase ATacticalGameState::GetGamePhase()
{ //Crash reported
	return EGamePhase();
}

bool ATacticalGameState::SetGamePhase_Validate(EGamePhase NewGamePhase)
{
	return true;
}

void ATacticalGameState::SetGamePhase_Implementation(EGamePhase NewGamePhase)
{
	GamePhase = NewGamePhase;
}

bool ATacticalGameState::SetTeamPlaying_Validate(int TeamNumber)
{
	return true;
}

void ATacticalGameState::SetTeamPlaying_Implementation(int TeamNumber)
{
	TeamPlaying = TeamNumber;
}

bool ATacticalGameState::SetNumberOfPlayers_Validate(int NewNumber)
{
	return true;
}

void ATacticalGameState::SetNumberOfPlayers_Implementation(int NewNumber)
{
	NumberOfPlayers = NewNumber;
}



FLinearColor ATacticalGameState::GetTeamColour(int TeamNumber)
{
	return TeamColours[TeamNumber - 1];
}
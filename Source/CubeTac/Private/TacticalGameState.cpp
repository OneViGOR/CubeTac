// Copyright 2019 James Vigor. All Rights Reserved.


#include "TacticalGameState.h"
#include "Engine.h"
#include "UnrealNetwork.h"

void ATacticalGameState::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("%d playing"), TeamPlaying);
}

void ATacticalGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(ATacticalGameState, GamePhase);
	DOREPLIFETIME(ATacticalGameState, TeamPlaying);
	DOREPLIFETIME(ATacticalGameState, NumberOfPlayers);

}

EGamePhase ATacticalGameState::GetGamePhase()
{
	return GamePhase;
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
	UE_LOG(LogTemp, Warning, TEXT("Team %d"), TeamNumber);
	if (TeamNumber > 0) {
		return TeamColours[FMath::Clamp(TeamNumber - 1, 0, 3)];
	}
	else {
		return FLinearColor(0,0,0,1);
	}
}
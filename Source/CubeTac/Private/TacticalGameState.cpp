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
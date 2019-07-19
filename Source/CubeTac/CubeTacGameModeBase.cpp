// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeTacGameModeBase.h"
#include "TacticalGameState.h"
#include "PlayerPawnC.h"

ACubeTacGameModeBase::ACubeTacGameModeBase() {

	GameStateClass = ATacticalGameState::StaticClass();
	PlayerControllerClass = ATacticalControllerBase::StaticClass();
	DefaultPawnClass = APlayerPawnC::StaticClass();
}

void ACubeTacGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetGameState<ATacticalGameState>()->SetGamePhase(EGamePhase::Phase_Lobby);

	Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController())->bTurn = true;
}

bool ACubeTacGameModeBase::PassTurn_Validate(ATacticalControllerBase* PlayerController)
{
	return true;
}

void ACubeTacGameModeBase::PassTurn_Implementation(ATacticalControllerBase* PlayerController) 
{

	PlayerController->bTurn = false;
	if ((TeamPlaying + 1) > NumberOfPlayers) {
		TeamPlaying = 1;
		if (GameStateReference->GetGamePhase() == EGamePhase::Phase_Portal) { //Crash reported
			GameStateReference->SetGamePhase(EGamePhase::Phase_Game);
		}
	}
	else {
		TeamPlaying++;
	}

	GameStateReference->SetTeamPlaying(TeamPlaying);
	ResetPlayerPieces(); //Pointed out by crash report
	PlayerControllers[TeamPlaying - 1]->bTurn = true;
}


void ACubeTacGameModeBase::ResetPlayerPieces()
{
	for (TActorIterator<AGridCharacterC> Itr(GetWorld()); Itr; ++Itr)
	{
		if (Itr->GetTeam() == TeamPlaying) {
			Itr->NewTurnStart();
		}
	}
}
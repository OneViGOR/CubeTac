// Copyright 2019 James Vigor. All Rights Reserved.


#include "CubeTacGameModeBase.h"
#include "TacticalGameState.h"
#include "TacticalControllerBase.h"
#include "PlayerPawnC.h"

ACubeTacGameModeBase::ACubeTacGameModeBase() {

	bStartPlayersAsSpectators = false;

	GameStateClass = ATacticalGameState::StaticClass();
	PlayerControllerClass = ATacticalControllerBase::StaticClass();
	DefaultPawnClass = APlayerPawnC::StaticClass();

}

void ACubeTacGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Start by saving a reference to the game state
	GameStateReference = GetGameState<ATacticalGameState>();
	GameStateReference->SetGamePhase(EGamePhase::Phase_Lobby);

	// First turn is taken by the server host
	Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController())->bTurn = true;
	
}

// New player joins the game, is given a pawn and a team
void ACubeTacGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	// Update game values to accomodate new player
	PlayerControllers.Add(Cast<ATacticalControllerBase>(NewPlayer));
	NumberOfPlayers++;
	Cast<ATacticalControllerBase>(NewPlayer)->Team = NumberOfPlayers;
	GameStateReference = GetGameState<ATacticalGameState>();
	GameStateReference->SetNumberOfPlayers(NumberOfPlayers);

	// Create tracker for the new player's portal state
	GameStateReference->SetPortalDestroyedState(NumberOfPlayers, false);

	UE_LOG(LogTemp, Warning, TEXT("Player Team %d"), Cast<ATacticalControllerBase>(NewPlayer)->Team);

	// Create a pawn for the new player to control
	FTransform SpawnLocation;
	SpawnLocation.SetLocation(FVector(0.0f, 0.0f, 75.0f));
	SpawnLocation.SetRotation(FQuat(FRotator(0, 0, 0)));
	SpawnLocation.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	NewPlayer->Possess(SpawnDefaultPawnAtTransform(NewPlayer, SpawnLocation));
}


// Game turn is passed to the next player
// - Validation
bool ACubeTacGameModeBase::PassTurn_Validate(ATacticalControllerBase* PlayerController)
{
	return true;
}

// - Implementation
void ACubeTacGameModeBase::PassTurn_Implementation(ATacticalControllerBase* PlayerController) 
{

	PlayerController->bTurn = false;
	if ((TeamPlaying + 1) > NumberOfPlayers) { // Last player in the order has ended their turn - return to the first player
		TeamPlaying = 1;
		if (GameStateReference->GetGamePhase() == EGamePhase::Phase_Portal) {  // Once every player has played in the Portal Placement phase, the Gameplay phase can commence
			GameStateReference->SetGamePhase(EGamePhase::Phase_Game);
		}
	}
	else {
		TeamPlaying++;  // If the player that just ended their turn was not the last player in the order, simply pass onto the player after them
	}

	GameStateReference->SetTeamPlaying(TeamPlaying);
	ResetPlayerPieces();  // Refresh the current player's pieces for the new turn
	PlayerControllers[TeamPlaying - 1]->bTurn = true;
}


// Refresh all of the current team's units for the start of their new turn
void ACubeTacGameModeBase::ResetPlayerPieces()
{
	for (TActorIterator<AGridUnit> Itr(GetWorld()); Itr; ++Itr)
	{
		if (Itr->GetTeam() == TeamPlaying) {
			Itr->NewTurnStart();
		}
	}
}


// Check whether the game should continue after a potentially game-ending event
// - Validation
bool ACubeTacGameModeBase::CheckGameEnded_Validate()
{
	return true;
}

// - Implementation
void ACubeTacGameModeBase::CheckGameEnded_Implementation()
{
	if (NumberOfPlayers > 1) {

		int TeamsStillPlaying = 0;

		for (int i = 0; i < NumberOfPlayers; i++) {
			// Define here the rules for determining whether or not a player is removed from the game
			// Use game state tracker for destroyed portals as it stays consistent regardless of players leaving the game
			TArray<bool> PortalStates = GameStateReference->GetPortalDestroyedStateArray();
			if (!PortalStates[i]) {
				TeamsStillPlaying++;
			}
		}

		if (TeamsStillPlaying == 0) {  //There is currently no way this should happen. Portals should be destroyed one-by-one.
			// Draw!
			UE_LOG(LogTemp, Error, TEXT("Draw. This should not be possible."));
		}

		else if (TeamsStillPlaying == 1) {  //One player remaining. That player is the victor.
			// Victory!
			UE_LOG(LogTemp, Error, TEXT("Victory!"));

			GameStateReference->SetGamePhase(EGamePhase::Phase_End);

			// If a player still has their portal when the game ends, they are victorious. Otherwise, they were defeated
			for (int i = 0; i < PlayerControllers.Num(); i++) {
				if (PlayerControllers[i]->GetPortal() != nullptr) {
					PlayerControllers[i]->EndGame(EVictoryState::State_Victorious);
				}
				else {
					PlayerControllers[i]->EndGame(EVictoryState::State_Defeated);
				}
			}
		}
		else {  //More than one player remain. The game continues.
			// Continue playing
			UE_LOG(LogTemp, Error, TEXT("Continue playing"))
		}

	}
}
// Copyright 2019 James Vigor. All Rights Reserved.


#include "CubeTacGameModeBase.h"
#include "TacticalGameState.h"
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
	
	GameStateReference = GetGameState<ATacticalGameState>();
	GameStateReference->SetGamePhase(EGamePhase::Phase_Lobby);

	Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController())->bTurn = true;
	
}

void ACubeTacGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	PlayerControllers.Add(Cast<ATacticalControllerBase>(NewPlayer));
	NumberOfPlayers++;
	Cast<ATacticalControllerBase>(NewPlayer)->Team = NumberOfPlayers - 1;
	GetGameState<ATacticalGameState>()->SetNumberOfPlayers(NumberOfPlayers);

	UE_LOG(LogTemp, Warning, TEXT("Player Team %d"), Cast<ATacticalControllerBase>(NewPlayer)->Team);

	FTransform SpawnLocation;
	SpawnLocation.SetLocation(FVector(0.0f, 0.0f, 75.0f));
	SpawnLocation.SetRotation(FQuat(FRotator(0, 0, 0)));
	SpawnLocation.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	NewPlayer->Possess(SpawnDefaultPawnAtTransform(NewPlayer, SpawnLocation));
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
		if (GameStateReference->GetGamePhase() == EGamePhase::Phase_Portal) {
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
		UE_LOG(LogTemp, Warning, TEXT("Character Team: %d"), Itr->GetTeam());
		UE_LOG(LogTemp, Warning, TEXT("Team Playing: %d"), TeamPlaying);
		if (Itr->GetTeam() + 1 == TeamPlaying) {
			UE_LOG(LogTemp, Warning, TEXT("Character Reset"));
			Itr->NewTurnStart();
		}
	}
}
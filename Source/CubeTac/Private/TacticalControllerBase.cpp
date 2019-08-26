// Copyright 2019 James Vigor. All Rights Reserved.


#include "TacticalControllerBase.h"
#include "CubeTacGameModeBase.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "Online.h"
#include "Blueprint/UserWidget.h"

ATacticalControllerBase::ATacticalControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetReplicates(true);
}

void ATacticalControllerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(ATacticalControllerBase, Team);
	DOREPLIFETIME(ATacticalControllerBase, SelectedUnit);
	DOREPLIFETIME(ATacticalControllerBase, bTurn);
	DOREPLIFETIME(ATacticalControllerBase, OwnedPortal);
}

void ATacticalControllerBase::BeginPlay()
{
	Super::BeginPlay();
	SetUpLobbyUI();

	FInputModeGameAndUI InputStruct;
	SetInputMode(InputStruct);
}


// Destroy session and return to main menu
void ATacticalControllerBase::LeaveGame()
{
	// WIP
}

// Returns this player to the main menu level with a reason provided
void ATacticalControllerBase::ClientReturnToMainMenuWithTextReason(const FText& ReturnReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Return client to menu"));
	Super::ClientReturnToMainMenuWithTextReason(ReturnReason);
}


// End this player's turn and pass to the next player in the turn order
// - Validation
bool ATacticalControllerBase::EndTurn_Validate()
{
	return true;
}

// - Implementation
void ATacticalControllerBase::EndTurn_Implementation()
{
	GetWorld()->GetAuthGameMode<ACubeTacGameModeBase>()->PassTurn(this);
	if (SelectedUnit != nullptr) {
		SelectedUnit->CancelAllNavigableLocations();
		SelectedUnit->CancelTargetting();
	}
}


// End the game with the victory state of this player
// - Validation
bool ATacticalControllerBase::EndGame_Validate( EVictoryState VictoryState)
{
	return true;
}

// - Implementation
void ATacticalControllerBase::EndGame_Implementation(EVictoryState VictoryState)
{
	if (GameWidgetReference != nullptr) {
		GameWidgetReference->DisplayEndGameScreen(VictoryState);
	}
}


// Runs when the player enters the lobby to set up the user interface
// - Validation
bool ATacticalControllerBase::SetUpLobbyUI_Validate() {
	return true;
}

// - Implementation
void ATacticalControllerBase::SetUpLobbyUI_Implementation() {
	if (LobbyWidget)
	{
		LobbyWidgetReference = CreateWidget<UUserWidget>(this, LobbyWidget);

		if (LobbyWidgetReference)
		{
			LobbyWidgetReference->AddToViewport();
		}
	}
}


// Runs when the player moves from the lobby to the game to set up the user interface
// - Validation
bool ATacticalControllerBase::SetUpGameUI_Validate() {
	return true;
}

// - Implementation
void ATacticalControllerBase::SetUpGameUI_Implementation() {
	if (LobbyWidgetReference != nullptr) {
		LobbyWidgetReference->RemoveFromParent();
	}

	if (GameWidget)
	{
		GameWidgetReference = CreateWidget<UGameInterfaceBase>(this, GameWidget);

		if (GameWidgetReference)
		{
			GameWidgetReference->AddToViewport();
		}
	}
}


// Runs when a player selects a new unit to get a reference to it and to update the game UI accordingly
// - Validation
bool ATacticalControllerBase::UnitSelected_Validate(AGridUnit* NewUnit)
{
	return true;
}

// - Implementation
void ATacticalControllerBase::UnitSelected_Implementation(AGridUnit* NewUnit)
{
	SelectedUnit = NewUnit;
	UISelect(NewUnit);
}


// Returns a reference to the portal controlled by this player
AUnit_Portal* ATacticalControllerBase::GetPortal()
{
	return OwnedPortal;
}

// Assign this player controller a new reference to the portal they own
 void ATacticalControllerBase::SetPortal(AUnit_Portal* NewPortal)
{
	OwnedPortal = NewPortal;
}

// Tell the game interface widget to update with the newly selected unit
 // - Validation
bool ATacticalControllerBase::UISelect_Validate(AGridUnit* NewUnit)
{
	return true;
}

// - Implementation
void ATacticalControllerBase::UISelect_Implementation(AGridUnit* NewUnit)
{
	GameWidgetReference->UnitSelected(NewUnit);
}
// Copyright 2019 James Vigor. All Rights Reserved.


#include "TacticalControllerBase.h"
#include "Engine.h"
#include "CubeTacGameModeBase.h"
#include "UnrealNetwork.h"
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
	DOREPLIFETIME(ATacticalControllerBase, SelectedCharacter);
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

bool ATacticalControllerBase::EndTurn_Validate()
{
	return true;
}

void ATacticalControllerBase::EndTurn_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("End Turn"));
	GetWorld()->GetAuthGameMode<ACubeTacGameModeBase>()->PassTurn(this);
	if (SelectedCharacter != nullptr) { //Pointed out by crash report
		SelectedCharacter->CancelAllNavigableLocations();
		SelectedCharacter->CancelTargetting();
	}
}

bool ATacticalControllerBase::SetUpLobbyUI_Validate() {
	return true;
}

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

bool ATacticalControllerBase::SetUpGameUI_Validate() {
	return true;
}

void ATacticalControllerBase::SetUpGameUI_Implementation() {
	if (LobbyWidgetReference->IsValidLowLevel()) {
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

bool ATacticalControllerBase::CharacterSelected_Validate(AGridCharacterC* NewCharacter)
{
	return true;
}

void ATacticalControllerBase::CharacterSelected_Implementation(AGridCharacterC* NewCharacter)
{
	SelectedCharacter = NewCharacter;
	UISelect(NewCharacter);
}

APortalC* ATacticalControllerBase::GetPortal()
{
	return OwnedPortal;
}

bool ATacticalControllerBase::UISelect_Validate(AGridCharacterC* NewCharacter)
{
	return true;
}

void ATacticalControllerBase::UISelect_Implementation(AGridCharacterC* NewCharacter)
{
	GameWidgetReference->CharacterSelected(NewCharacter);
}
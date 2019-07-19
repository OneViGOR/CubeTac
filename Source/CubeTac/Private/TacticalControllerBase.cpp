// Fill out your copyright notice in the Description page of Project Settings.


#include "TacticalControllerBase.h"
#include "CubeTacGameModeBase.h"
#include "UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

ATacticalControllerBase::ATacticalControllerBase()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	bReplicates = true;
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
		GameWidgetReference = CreateWidget<UUserWidget>(this, GameWidget);

		if (GameWidgetReference)
		{
			GameWidgetReference->AddToViewport();
		}
	}
}

void ATacticalControllerBase::CharacterSelected(AGridCharacterC* character)
{

}
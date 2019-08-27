// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Unit_Portal.h"
#include "GameInterfaceBase.h"
#include "AlertBoxBase.h"
#include "Online.h"
#include "GameFramework/PlayerController.h"
#include "TacticalControllerBase.generated.h"


class AGridUnit;

UCLASS()
class CUBETAC_API ATacticalControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	ATacticalControllerBase();
	
	/**
	*   WIDGET REFERENCES
	*/

	// Reference to lobby widget asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
		TSubclassOf<class UUserWidget> LobbyWidget;

	// Pointer to instance of lobby widget once created
	UPROPERTY()
		UUserWidget* LobbyWidgetReference;

	// Reference to game interface widget asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
		TSubclassOf<class UGameInterfaceBase> GameWidget;

	// Pointer to instance of game interface widget once created
	UPROPERTY()
		UGameInterfaceBase* GameWidgetReference;
protected:


public:

	/**
	*   FUNCTIONS
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Destroy session and return to main menu
	UFUNCTION(BlueprintCallable)
		void LeaveGame();

	/**
	*   OVERRIDE - Returns this player to the main menu level with a reason provided
	*
	*	@Param		ReturnReason		The reason for returning the player to the main menu
	*/
	void ClientReturnToMainMenuWithTextReason(const FText& ReturnReason) override;

	// End this player's turn and pass to the next player in the turn order
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void EndTurn();

	/**
	*   End the game with the victory state of this player
	*
	*	@Param		VictoryState		An enumeration that states whether this player won, lost or drew the game
	*/
	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
		void EndGame(EVictoryState VictoryState);

	// Runs when the player enters the lobby to set up the user interface
	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpLobbyUI();

	// Runs when the player moves from the lobby to the game to set up the user interface
	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpGameUI();

	/**
	*   Runs when a player selects a new unit to get a reference to it and to update the game UI accordingly
	*
	*	@Param		NewUnit		A pointer to the newly selected unit
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void UnitSelected(AGridUnit* NewUnit);

	// Returns a reference to the portal controlled by this player
	UFUNCTION(BlueprintCallable)
		AUnit_Portal* GetPortal();

	/**
	*   Assign this player controller a new reference to the portal they own
	*
	*	@Param		NewPortal		A pointer to the new portal
	*/
	UFUNCTION(BlueprintCallable)
		void SetPortal(AUnit_Portal* NewPortal);

	/**
	*   Tell the game interface widget to update with the newly selected unit
	*
	*	@Param		NewUnit		A pointer to the newly selected unit
	*/
	UFUNCTION(Client, Reliable, WithValidation)
		void UISelect(AGridUnit* NewUnit);



	/**
	*   VARIABLES
	*/

	// The player's team as an integer 1-4
	UPROPERTY(BlueprintReadOnly, Replicated)
		int Team;

	// A pointer to the unit the player is currently controlling
	UPROPERTY(BlueprintReadOnly, Replicated)
		AGridUnit* SelectedUnit;

	// A pointer to the portal for this player's team
	UPROPERTY(BlueprintReadOnly, Replicated)
		AUnit_Portal* OwnedPortal;

	// True if it is currently this player's turn. False if another player is taking their turn
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bTurn;

};

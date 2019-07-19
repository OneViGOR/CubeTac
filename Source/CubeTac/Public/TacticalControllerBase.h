// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalC.h"
#include "GameFramework/PlayerController.h"
#include "TacticalControllerBase.generated.h"

class AGridCharacterC;

UCLASS()
class CUBETAC_API ATacticalControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	ATacticalControllerBase();
	
	//Lobby UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
		TSubclassOf<class UUserWidget> LobbyWidget;
	UPROPERTY()
		UUserWidget* LobbyWidgetReference;

	//Game UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
		TSubclassOf<class UUserWidget> GameWidget;
	UPROPERTY()
		UUserWidget* GameWidgetReference;
protected:


public:

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void EndTurn();

	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpLobbyUI();
	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpGameUI();
	UFUNCTION()
		void CharacterSelected(AGridCharacterC* CharacterParam);

	UPROPERTY(BlueprintReadOnly, Replicated)
		int Team;
	UPROPERTY(BlueprintReadOnly, Replicated)
		AGridCharacterC* SelectedCharacter;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bTurn;
	UPROPERTY(BlueprintReadOnly, Replicated)
		APortalC* OwnedPortal;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalC.h"
#include "GameFramework/PlayerController.h"
#include "TacticalControllerC.generated.h"

class AGridCharacterC;

UCLASS()
class CUBETAC_API ATacticalControllerC : public APlayerController
{
	GENERATED_BODY()

public:
	ATacticalControllerC();
	
	// Reference UMG Asset in the Editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
		TSubclassOf<class UUserWidget> wMainMenu;

	// Variable to hold the widget After Creating it.
	UUserWidget* MyMainMenu;

protected:


public:

	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpGameUI();
	UFUNCTION()
		void CharacterSelected(AGridCharacterC* CharacterParam);

	int Team;
	AGridCharacterC* SelectedCharacter;
	bool bTurn;
	APortalC* OwnedPortal;
};

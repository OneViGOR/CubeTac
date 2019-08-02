// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PortalC.h"
#include "GameInterfaceBase.h"
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
		TSubclassOf<class UGameInterfaceBase> GameWidget;
	UPROPERTY()
		UGameInterfaceBase* GameWidgetReference;
protected:


public:

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void EndTurn();

	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpLobbyUI();
	UFUNCTION(Client, Reliable, WithValidation)
		void SetUpGameUI();
	UFUNCTION(Server, Reliable, WithValidation)
		void CharacterSelected(AGridCharacterC* NewCharacter);
	UFUNCTION(BlueprintCallable)
		APortalC* GetPortal();
	UFUNCTION(Client, Reliable, WithValidation)
		void UISelect(AGridCharacterC* NewCharacter);


	UPROPERTY(BlueprintReadOnly, Replicated)
		int Team;
	UPROPERTY(BlueprintReadOnly, Replicated)
		AGridCharacterC* SelectedCharacter;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bTurn;
	UPROPERTY(BlueprintReadOnly, Replicated)
		APortalC* OwnedPortal;
};

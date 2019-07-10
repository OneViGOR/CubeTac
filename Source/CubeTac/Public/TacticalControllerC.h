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
	void CharacterSelected(AGridCharacterC* character);

	int Team;
	AGridCharacterC* SelectedCharacter;
	bool bTurn;
	APortalC* OwnedPortal;
};

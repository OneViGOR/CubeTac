// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TacticalGameState.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8 {
	Phase_Lobby			UMETA(DisplayName = "Lobby"),
	Phase_Portal		UMETA(DisplayName = "Portal Placement"),
	Phase_Game			UMETA(DisplayName = "Gameplay")
};

UCLASS()
class CUBETAC_API ATacticalGameState : public AGameStateBase
{
	GENERATED_BODY()
	

public:
	TArray<FLinearColor> TeamColours;
	UPROPERTY(Replicated)
	EGamePhase GamePhase;
	UPROPERTY(Replicated)
	int TeamPlaying;
	UPROPERTY(Replicated)
	int NumberOfPlayers;
};

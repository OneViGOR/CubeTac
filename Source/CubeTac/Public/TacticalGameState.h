// Copyright 2019 James Vigor. All Rights Reserved.

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
	

protected:
	UPROPERTY(BlueprintReadOnly)
		TArray<FLinearColor> TeamColours = { FLinearColor(0.0f, 0.309435f, 1.0f, 1.0f), FLinearColor(1.0f, 0.013363f, 0.0f, 1.0f), FLinearColor(0.073495f, 0.696875f, 0.0f, 1.0f), FLinearColor(1.0f, 0.425576f, 0.0f, 1.0f) };
	UPROPERTY(Replicated)
		EGamePhase GamePhase;
	UPROPERTY(Replicated, BlueprintReadOnly)
		int TeamPlaying = 1;
	UPROPERTY(Replicated)
		int NumberOfPlayers;

public:
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
		EGamePhase GetGamePhase();
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void SetGamePhase(EGamePhase NewGamePhase);
	UFUNCTION(Server, Reliable, WithValidation)
		void SetTeamPlaying(int TeamNumber);
	UFUNCTION(Server, Reliable, WithValidation)
		void SetNumberOfPlayers(int NewNumber);

	UFUNCTION()
		FLinearColor GetTeamColour(int TeamNumber);

};

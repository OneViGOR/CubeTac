// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Unit_Portal.h"
#include "GridUnit.h"
#include "CubeTacGameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "GameInterfaceBase.generated.h"


UCLASS()
class CUBETAC_API UGameInterfaceBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	*   FUNCTIONS
	*/

	// Function called to update the interface when a unit is selected.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void UnitSelected(AGridUnit* Unit);

	// Function called to add buttons that allow a selected portal to spawn units.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void GenerateSpawnButtons(AUnit_Portal* Portal);

	// Function called to add buttons that allow a selected unit to use its abilities.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void GenerateAbilityButtons(AGridUnit* Unit);

	// Function called to display a message at the end of the game, telling the user whether or not they were victorious.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void DisplayEndGameScreen(EVictoryState VictoryState);
};
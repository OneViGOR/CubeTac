// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PortalC.h"
#include "GridCharacterC.h"
#include "Blueprint/UserWidget.h"
#include "GameInterfaceBase.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API UGameInterfaceBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void CharacterSelected(AGridCharacterC* Character);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void GenerateSpawnButtons(APortalC* Portal);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void GenerateAbilityButtons(AGridCharacterC* Character);
};
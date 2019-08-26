// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlertBoxBase.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API UAlertBoxBase : public UUserWidget
{
	GENERATED_BODY()

public:

	// Provides the text that is displayed to the user. If this is not used, the alert will display as empty.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void InitialiseAlert(const FText& AlertText);
};

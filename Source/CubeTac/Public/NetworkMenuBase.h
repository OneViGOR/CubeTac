// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetworkMenuBase.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API UNetworkMenuBase : public UUserWidget
{
	GENERATED_BODY()

public:
	void StartGameSession();
};

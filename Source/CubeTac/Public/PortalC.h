// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridCharacterC.h"
#include "PortalC.generated.h"

UCLASS()
class CUBETAC_API APortalC : public AGridCharacterC
{
	GENERATED_BODY()
	
public:
	void SpawnCharacter(AActor_MapTile* SpawnTile);
};

// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridCharacterC.h"
#include "SpawnlingC.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API ASpawnlingC : public AGridCharacterC
{
	GENERATED_BODY()

public:
	ASpawnlingC();
	
	void Ability1(AMapTile* TargetTile) override;
	void Ability2(AMapTile* TargetTile) override;
	void Ability3(AMapTile* TargetTile) override;

private:
	UParticleSystem* AttackParticles;
	UParticleSystem* HealParticles;
};

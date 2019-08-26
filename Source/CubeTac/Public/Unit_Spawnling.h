// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Spawnling.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API AUnit_Spawnling : public AGridUnit
{
	GENERATED_BODY()

public:
	AUnit_Spawnling();
	
	/**
	*	OVERRIDE - Replace empty default ability functions with new ones
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	void Ability1(AMapTile* TargetTile) override;
	void Ability2(AMapTile* TargetTile) override;
	void Ability3(AMapTile* TargetTile) override;

private:
	// Particle system to be used by the spawnling's offensive ability
	UParticleSystem* AttackParticles;

	// Particle system to be used by the spawnling's healing ability
	UParticleSystem* HealParticles;
};

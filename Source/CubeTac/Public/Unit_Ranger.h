// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Ranger.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API AUnit_Ranger : public AGridUnit
{
	GENERATED_BODY()

public:
	AUnit_Ranger();

	/**
	*	OVERRIDE - Replace empty default ability functions with new ones
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	void BaseAbility(AMapTile* TargetTile) override;

private:
	// Particle system to be used by the ranger's melee attack ability
	UParticleSystem* AttackParticles;
};

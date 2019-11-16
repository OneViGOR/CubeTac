// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Genie.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API AUnit_Genie : public AGridUnit
{
	GENERATED_BODY()
	
public:
	AUnit_Genie();

	/**
	*	OVERRIDE - Replace empty default ability functions with new ones
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	void BaseAbility(AMapTile* TargetTile) override;
	void Ability1(AMapTile* TargetTile) override;

private:
	// Particle system to be used by the genie's melee attack ability
	UParticleSystem* AttackParticles;

	// Particle system to be used by the genie's healing ability
	UParticleSystem* HealParticles;
};

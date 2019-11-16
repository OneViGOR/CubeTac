// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Warden.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API AUnit_Warden : public AGridUnit
{
	GENERATED_BODY()
	
public:
	AUnit_Warden();

	/**
	*	OVERRIDE - Replace empty default ability functions with new ones
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	void BaseAbility(AMapTile* TargetTile) override;

private:
	// Particle system to be used by the warden's melee attack ability
	UParticleSystem* AttackParticles;
};

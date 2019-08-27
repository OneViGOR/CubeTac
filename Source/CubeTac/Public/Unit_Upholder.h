// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Upholder.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API AUnit_Upholder : public AGridUnit
{
	GENERATED_BODY()
	
public:
	AUnit_Upholder();

	/**
	*	OVERRIDE - Replace empty default ability function with new one
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	void Ability1(AMapTile* TargetTile) override;

private:
	// Particle system to be used by the upholder's offensive ability
	UParticleSystem* AttackParticles;

};

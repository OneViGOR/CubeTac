// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Webber.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API AUnit_Webber : public AGridUnit
{
	GENERATED_BODY()

public:
	AUnit_Webber();

	/**
	*	OVERRIDE - Replace empty default ability functions with new ones
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	void BaseAbility(AMapTile* TargetTile) override;
	void Ability1(AMapTile* TargetTile) override;
	void Ability2(AMapTile* TargetTile) override;
	void Ability3(AMapTile* TargetTile) override;


	/**
	*	OVERRIDE - Replace empty default ability rule functions with new ones
	*
	*	@Param		TargetTile		The tile that the ability will be directed towards
	*/
	bool AbilityRule1(AMapTile* TargetTile) override;
	bool AbilityRule2(AMapTile* TargetTile) override;

private:
	// Particle system to be used by the webber's offensive ability
	UParticleSystem* AttackParticles;
};

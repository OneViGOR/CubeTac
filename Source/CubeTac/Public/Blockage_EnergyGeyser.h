// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlockageC.h"
#include "Blockage_EnergyGeyser.generated.h"

/**
 * 
 */
UCLASS()
class CUBETAC_API ABlockage_EnergyGeyser : public ABlockageC
{
	GENERATED_BODY()
	
public:
	ABlockage_EnergyGeyser();

protected:
	class UParticleSystemComponent* GeyserParticleJet;
};

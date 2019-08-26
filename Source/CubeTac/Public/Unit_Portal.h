// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridUnit.h"
#include "Unit_Portal.generated.h"

// A structure containing a unit class and energy cost used for spawning new units
USTRUCT(BlueprintType)
struct FUnitBuyData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
		TSubclassOf<AGridUnit> UnitClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
		int Price;

	// Constructors
	FUnitBuyData() {
		UnitClass = nullptr;
		Price = 1;
	}

	FUnitBuyData(TSubclassOf<AGridUnit> ClassParam, int PriceParam) {
		UnitClass = ClassParam;
		Price = PriceParam;
	}
};

UCLASS()
class CUBETAC_API AUnit_Portal : public AGridUnit
{
	GENERATED_BODY()
	
public:
	AUnit_Portal();

protected:
	/**
	*   FUNCTIONS
	*/

	virtual void BeginPlay() override;


	/**
	*   COMPONENTS
	*/

	// Team-coloured rift that glows within the portal frame
	UPROPERTY()
		UStaticMeshComponent* RiftMesh;

	// Looping audio component for portal ambient sound
	UPROPERTY()
		UAudioComponent* AmbientHum;


	/**
	*   VARIABLES
	*/

	// A pointer to the actor that manages to game map
	UPROPERTY()
		AGameMap* GameMapReference;

	// An array containing all of the units portals can spawn and how much energy each on costs
	UPROPERTY(BlueprintReadOnly)
		TArray<FUnitBuyData> SpawnList;

	// The information about the unit being prepared for spawn
	UPROPERTY(BlueprintReadOnly)
		FUnitBuyData SelectedSpawn;

	// The number of tiles out from the portal that units can be spawned
	UPROPERTY()
		int SpawnRange;

	// The amount of energy the portal gains each turn, unlike other units which start each turn with their maximum amount
	UPROPERTY()
		int EnergyPerTurn;

public:
	/**
	*   FUNCTIONS
	*/

	// OVERRIDE - Portal death does same as standard units, but also checks whether the game should end as a result of a team being eliminated
	void Death() override;

	/**
	*	Spawn the selected unit type on a given tile
	*
	*	@Param		SpawnTile		The tile the new unit will spawn upon
	*/
	UFUNCTION()
		void SpawnUnit(AMapTile* SpawnTile);

	/**
	*	Player selects which type of unit they wish to spawn
	*
	*	@Param		SelectedData		The structure containing the class and energy cost of the unit to be spawned
	*/
	UFUNCTION(BlueprintCallable)
		void SelectSpawnUnit(FUnitBuyData SelectedData);

	// OVERRIDE - Deselects this unit - works as normal but also clears SelectedSpawn
	void DeselectUnit() override;
	
	// OVERRIDE - Adds energy to the portal's capacity at the start of its turn rather than resetting it to maximum value
	void NewTurnStart_Implementation() override;
};
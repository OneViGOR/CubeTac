// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockageC.h"
#include "SkySphereC.h"
#include "Runtime/Engine/Classes/Components/ChildActorComponent.h"
#include "MapTile.generated.h"

class AGridUnit;
class ATileMarker;
class AGameMap;

// An enumeration to define how easily tiles can be reached by the selected unit
UENUM(BlueprintType)
enum class ENavigationEnum : uint8 {
	Nav_Safe			UMETA(DisplayName = "Safe"),
	Nav_Dangerous		UMETA(DisplayName = "Dangerous"),
	Nav_Unreachable		UMETA(DisplayName = "Unreachable")
};

UCLASS()
class CUBETAC_API AMapTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapTile();

protected:
	/**
	*   FUNCTIONS
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	*   COMPONENTS
	*/

	// Root component provides transform data
	UPROPERTY()
		USceneComponent* SceneRoot;

	// Cube mesh on the top of the tile
	UPROPERTY()
		UStaticMeshComponent* CapMesh;

	// Cube mesh that forms the body of the tile
	UPROPERTY()
		UStaticMeshComponent* ShaftMesh;

	// Glowing highlight marker
	UPROPERTY()
		UChildActorComponent* TileMarker;
	
	/**
	*   VARIABLES
	*/

	// This tile's coordinates in the game map grid - not world space
	UPROPERTY()
		FVector2D Coordinates;

	// A pointer to the unit occupying this tile
	UPROPERTY(BlueprintReadWrite, Replicated)
		AGridUnit* OccupyingUnit;

	// A pointer to the blockage currently on this tile
	UPROPERTY(Replicated)
		ABlockageC* Blockage;

	// A pointer to the game map actor that spawned this tile
	UPROPERTY(Replicated)
		AGameMap* GameMapReference;

	// The number of movement points required to leave this tile if there are no blockages
	UPROPERTY()
		int MovementCost;
	
	// True if this tile is void, leaving an empty space in the game map
	UPROPERTY(Replicated)
		bool bVoid;

private:
	// A reference to the data table that holds the enivornmental colour palette
	class UDataTable* EnvironmentData;

public:	
	/**
	*   FUNCTIONS
	*/

	/**
	*   Set this tile up with a reference to the game map and coordinates within it
	*
	*	@Param		Map			Pointer to game map actor
	*	@Param		X			X coordinate in map grid
	*	@Param		Y			Y coordinate in map grid
	*/
	void SetCoordinates(AGameMap* Map, int X, int Y);

	/**
	*   Set this tile's void state. If true, its space in the map will appear and act unoccupied
	*
	*	@Param		bVoidParam		Whether or not this tile should act as void
	*/
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SetVoid(bool bVoidParam);

	/**
	*	Set the appearance of this map tile based on user preference
	*
	*	@Param		EnvironmentParam		An enumeration to determine the environmental colour scheme
	*/
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SetAtmosphere(EEnvironmentEnum Environment);
	
	/**
	*	Assign a reference to the unit occupying this tile
	*
	*	@Param		NewOccupier		A pointer to the occupying unit
	*/
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SetOccupyingUnit(AGridUnit* NewOccupier);

	/**
	*	Cast a short line trace downwards to find a tile below the given point. Returns a reference to the tile, if found
	*
	*	@Param		Start		The world location of the point the line trace originates from
	*/
	AMapTile* LineTraceForTile(FVector Start);

	// Returns up to four tiles orthogonally adjacent to this one
	TArray<AMapTile*> GetFourNeighbouringTiles();

	// Update the colour and visibility of the tile marker based on known variables
	void SetHighlightMaterial();

	// Contains behaviour for when the tile is clicked during the Gameplay phase
	void ClickedInGamePhase();

	// Contains behaviour for when the tile is clicked during the Portal Placement phase
	void ClickedInPortalPlacementPhase();

	/**
	*	Deferred function that runs when a player cursor hovers over this tile
	*
	*	@Param		Actor		Reference to the actor now under the mouse cursor (in this case, it should be the tile instance)
	*/
	UFUNCTION()
		void OnBeginMouseOver(AActor* Actor);

	/**
	*	Deferred function that runs when a player cursor is no longer hovering over this tile
	*
	*	@Param		Actor		Reference to the actor that was under under the mouse cursor (in this case, it should be the tile instance)
	*/
	UFUNCTION()
		void OnEndMouseOver(AActor* Actor);

	/**
	*	Deferred function that runs when a player clicks on this tile
	*
	*	@Param		Actor				Reference to the actor that was clicked (in this case, it should be the tile instance)
	*	@Param		ButtonPressed		The input button that was pressed to trigger this event
	*/
	UFUNCTION()
		void OnMouseClicked(AActor* Actor, FKey ButtonPressed);


	//Getters and Setters

	// Returns the movement energy it takes to leave this tile, excluding that added by any blockage on this tile
	int GetBaseMovementCost();

	// Returns the total movement energy it takes to leave this tile, including that added by any blockage on this tile
	int GetTotalMovementCost();

	// Returns a pointer to the unit currently occupying this tile, if there is one
	AGridUnit* GetOccupyingUnit();

	// Sets this tile's reference to its new assigned blockage
	void SetBlockage(ABlockageC* NewBlockage);

	// Returns a pointer to the blockage currently on this tile, if there is one
	ABlockageC* GetBlockage();

	// Returns true if this tile is considered void. Otherwise, returns false
	bool GetVoid();


	/**
	*   VARIABLES
	*/

	// An enumeration to define how easily this tile can be reached by the selected unit
	ENavigationEnum ECurrentlyNavigable = ENavigationEnum::Nav_Unreachable;

	// Set to true if this tile is a valid target for a unit preparing to use an ability. Otherwise, set to false
	bool bTargetable;

	// True if this tile has a cursor hovering over it on the local client. Otherwise, false
	bool bMouseOver;
};

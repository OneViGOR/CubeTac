// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MapTile.h"
#include "SkySphereC.h"
#include "GameFramework/Actor.h"
#include "GameMap.generated.h"

// Structure that holds all data necessary to spawn and reference a tile.
USTRUCT(BlueprintType)
struct FTileDataC {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		AMapTile* Tile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		float Height;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		bool bVoid;

	// Constructor
	FTileDataC() {
		Tile = nullptr;
		Height = 0.0f;
		bVoid = false;
	}
};

// Structure that holds a 1-dimensional array of tiles, representing a row on the map grid, contained as a single entity.
USTRUCT(BlueprintType)
struct FTileRow {
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
		TArray<FTileDataC> Tiles;

	// Constructor
	FTileRow() {

	}
};

// Structure that holds highest, lowest and average heights. Used for determining the maximum and minimum heights that a new tile can spawn at.
USTRUCT(BlueprintType)
struct FNeighbouringTileHeights {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Average")
		float AverageHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundaries")
		float Tallest;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundaries")
		float Shortest;

	// Constructor
	FNeighbouringTileHeights() {
		AverageHeight = 0.0f;
		Tallest = 0.0f;
		Shortest = 0.0f;
	}
};

UCLASS()
class CUBETAC_API AGameMap : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGameMap();

protected:
	/**
	*   FUNCTIONS
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Map Generation

	// Runs the map building algorithm with existing settings. To define new settings, use the RegenerateMap function
	UFUNCTION()
		void GenerateMap();

	/**
	*	Runs the map building algorithm with new settings
	*
	*	@Param		NewSeed					Seed to use for procedural map generation
	*	@Param		RowsParam				The number of rows in the map grid
	*	@Param		ColumnsParam			The number of columns in the map grid
	*	@Param		TileHeightParam			The maximum height allowed for each tile
	*	@Param		SlopeHeightParam	    The maximum amount by which the height of a tile can deviate from its neighbours
	*	@Param		VoidWeightParam			The chance of each tile generating as a void space
	*	@Param		BlockageWeightParam	    The chance of each tile generating with a blockage on top
	*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void RegenerateMap(int NewSeed, int RowsParam, int ColumnsParam, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam, float GeyserWeightParam);
	
	// Generates tile data for the map and spawns the tiles accordingly
	UFUNCTION()
		void BuildMapGrid();

	// Generates tile data for BuildMapGrid function
	UFUNCTION()
		void CreateGridArray();

	// Destroys all actors comprising the map and resets map data in order for the map to be generated again
	UFUNCTION()
		void DestroyMap();

	/**
	*	Spawns a blockage actor and assigns it to the appropriate map tile
	*
	*	@Param		Tile				The tile that the blockage will spawn on
	*	@Param		BlockageClass		A class reference to the type of blockage to be spawned
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void PlaceBlockageOnTile(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass);

	/**
	*	Returns a structure containing highest, lowest and average heights of any existing tiles orthogonally adjacent to the given coordinates
	*
	*	@Param		TileX			The X coordinate of the tile to start from
	*	@Param		TileY			The Y coordinate of the tile to start from
	*/
	UFUNCTION()
		FNeighbouringTileHeights GetNeighbouringTileHeights(int TileX, int TileY);
	
	/**
	*	Used by the GetNeighbouringTileHeights function to keep a track of the tallest and shortest tile heights found
	*
	*	@Param		CurrentTileHeight		The height of the tile currently being examined
	*	@Param		TallestTile				The tallest height found prior to the current tile
	*	@Param		ShortestTile			The shortest height found prior to the current tile
	*/
	UFUNCTION()
		void ConsolidateHeightChanges(float& CurrentTileHeight, float& TallestTile, float& ShortestTile);

	/**
	*	Returns a structure containing the data regarding the tile at the given coordinates
	*
	*	@Param		TileX			The X coordinate of the tile to start from
	*	@Param		TileY			The Y coordinate of the tile to start from
	*/
	UFUNCTION()
		FTileDataC GetTileDataAtCoordinates(int TileX, int TileY);

	/**
	*	Checks to see if a tile has been spawned at the given coordinates
	*
	*	@Param		TileX			The X coordinate of the tile to start from
	*	@Param		TileY			The Y coordinate of the tile to start from
	*/
	UFUNCTION()
		bool DoesTileExist(int TileX, int TileY);

	/**
	*	Set the appearance of the map tiles based on user preference
	*
	*	@Param		EnvironmentParam		An enumeration to determine the environmental colour scheme
	*/
	UFUNCTION(BlueprintCallable)
		void SetAtmosphere(EEnvironmentEnum EnvironmentParam);


	/**
	*   VARIABLES
	*/

	// Array of tile rows, representing the entire grid
	UPROPERTY()
		TArray<FTileRow> TileGrid;


	// The following are variables for use during map generation - Do not use elsewhere

	// Contains data for a column during generation - Not final data
	UPROPERTY()
		TArray<FTileDataC> ColumnSetupArray;

	// The index of the row currently being generated
	UPROPERTY()
		int RowBeingSetUp;

	// Whether or not the map is currently being generated
	UPROPERTY()
		bool bSetUpInProgress;

	// The map generation seed mutates many times during map generation - This variable tracks these changes without overwriting the record of the original seed
	UPROPERTY()
		int WorkingSeed;


	// The following are settings that determine how the map is generated

	// The initial seed used to generate the map. This dictates the general structure of the map, which is built upon by subsequent settings
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		int InitialSeed = 0;

	// The number of rows of tiles in the map grid
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		int Rows;

	// The number of columns of tiles in the map grid
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		int Columns;

	//The maximum height allowed for each tile		
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		float MaxTileHeight;

	//The maximum amount by which the height of a tile can deviate from its neighbours
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		float MaxSlopeHeight;

	//The chance of each tile generating as a void space
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		float VoidWeight;

	//The chance of each tile generating with a blockage on top
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		float BlockageWeight;

	//The chance of each tile generating with an energy geyser on top
	UPROPERTY(EditAnywhere, Category = "Map|Settings")
		float GeyserWeight;

	//An enumeration to determine the environmental colour scheme
	UPROPERTY()
		EEnvironmentEnum EnvironmentType;
public:
	/**
	*   COMPONENTS
	*/

	// Root component provides transform data
	UPROPERTY()
		USceneComponent* SceneRoot;
};
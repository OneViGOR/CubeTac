// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor_MapTile.h"
#include "SkySphereC.h"
#include "GameFramework/Actor.h"
#include "GameMap.generated.h"

USTRUCT(BlueprintType)
struct FTileDataC {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		AActor_MapTile* Tile;
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

USTRUCT(BlueprintType)
struct FTileRow {
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
		TArray<FTileDataC> Tiles;

	// Constructor
	FTileRow() {

	}
};

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
		Tallest = false;
		Shortest = false;
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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Map Generation
	UFUNCTION()
		void GenerateMap();
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void RegenerateMap(int NewSeed, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam);
	UFUNCTION()
		void BuildMapGrid();
	UFUNCTION()
		void CreateGridArray();
	UFUNCTION()
		void PlaceMidpoint();
	UFUNCTION()
		void DestroyMap();

	UFUNCTION()
		FNeighbouringTileHeights GetNeighbouringTileHeights(int TileX, int TileY);
	UFUNCTION()
		void ConsolidateHeightChanges(float& CurrentTileHeight, float& TallestTile, float& ShortestTile);
	UFUNCTION()
		FTileDataC GetTileDataAtCoordinates(int TileX, int TileY);
	UFUNCTION()
		bool DoesTileExist(int TileX, int TileY);

	UFUNCTION(BlueprintCallable)
		void SetAtmosphere(EEnvironmentEnum EnvironmentParam);

	TArray<FTileRow> TileGrid;

	//Variables for Setting Up the Map
	TArray<FTileDataC> ColumnSetupArray;
	int RowBeingSetUp;
	bool bSetUpInProgress;
	int WorkingSeed;

	EEnvironmentEnum EnvironmentType;

	// Generation Settings
	UPROPERTY(EditAnywhere)
		int InitialSeed = 0;
	UPROPERTY(EditAnywhere)
		int Rows = 10;
	UPROPERTY(EditAnywhere)
		int Columns = 10;
	UPROPERTY(EditAnywhere)
		float MaxTileHeight = 5.0f;
	UPROPERTY(EditAnywhere)
		float MaxSlopeHeight = 0.5f;
	UPROPERTY(EditAnywhere)
		float VoidWeight;
	UPROPERTY(EditAnywhere)
		float BlockageWeight;

public:
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;
	UPROPERTY()
		USceneComponent* Midpoint;
};

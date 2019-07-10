// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor_MapTile.h"
#include "SkySphereC.h"
#include "GameFramework/Actor.h"
#include "GameMap.generated.h"

USTRUCT(BlueprintType)
struct FTileData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		AActor_MapTile* Tile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		float Height;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
		bool bVoid;

	// Constructor
	FTileData() {
		Tile = nullptr;
		Height = 0.0f;
		bVoid = false;
	}
};

USTRUCT(BlueprintType)
struct FTileRow {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
		TArray<FTileData> Tiles;

	// Constructor
	FTileRow() {
		
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
	UFUNCTION(Server, Reliable, WithValidation)
		void RegenerateMap(int NewSeed, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam);
	UFUNCTION()
		void BuildMapGrid();
	UFUNCTION()
		void PlaceMidpoint();
	UFUNCTION()
		void DestroyMap();

	UFUNCTION()
		void SetAtmosphere(EEnvironmentEnum EnvironmentParam);

	TArray<FTileRow> TileGrid;

	//Variables for Setting Up the Map
	TArray<FTileData> ColumnSetupArray;
	int RowBeingSetUp;
	bool bSetUpInProgress;
	int WorkingSeed;
	
	EEnvironmentEnum EnvironmentType;

	// Generation Settings
	int InitialSeed;
	int Rows;
	int Columns;
	float MaxTileHeight;
	float MaxSlopeHeight;
	float VoidWeight;
	float BlockageWeight;

public:
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;
	UPROPERTY()
		USceneComponent* Midpoint;
};

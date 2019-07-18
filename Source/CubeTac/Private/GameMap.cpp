// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMap.h"

UFUNCTION(Server)
template <class T>
T* PlaceBlockageOnTile(AActor_MapTile* Tile)
{
	Tile->Blockage = GetWorld()->SpawnActor<T>(T::StaticClass(), NAME_None, Tile->GetActorLocation(), Tile->GetActorRotation(), NULL, false, false, Owner, Instigator);
	return actor;
}

// Sets default values
AGameMap::AGameMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;
	
	Midpoint = CreateDefaultSubobject<USceneComponent>(TEXT("Midpoint"));
	Midpoint->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void AGameMap::BeginPlay()
{
	Super::BeginPlay();
	WorkingSeed = InitialSeed;
	GenerateMap();
}

void AGameMap::GenerateMap()
{
	if (this->HasAuthority()) {
		BuildMapGrid();
		PlaceMidpoint();
		for (TActorIterator<AActor_MapTile> Itr(GetWorld()); Itr; ++Itr)
		{
			FRandomStream GenerationStream;
			GenerationStream.Initialize(WorkingSeed);
			float BlockageRoll = GenerationStream.FRandRange(0.0f, 1.0f);
			if (BlockageRoll < BlockageWeight && !Itr->GetVoid()) {
				//PLACE BLOCKAGE ON TILE
			}

			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));
		}
	}
}

bool AGameMap::RegenerateMap_Validate(int NewSeed, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam)
{
	return true;
}

void AGameMap::RegenerateMap_Implementation(int NewSeed, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam)
{
	DestroyMap();
	WorkingSeed = FMath::Clamp(NewSeed, 0, 999999);
	MaxTileHeight = TileHeightParam;
	MaxSlopeHeight = SlopeHeightParam;
	VoidWeight = VoidWeightParam;
	BlockageWeight = BlockageWeightParam;
	GenerateMap();
}

void AGameMap::BuildMapGrid()
{
	CreateGridArray();
	UE_LOG(LogTemp, Warning, TEXT("Array Complete"));
	for (int i = 0; i < TileGrid.Num(); i++) {
		UE_LOG(LogTemp, Warning, TEXT("Rows: %d"), TileGrid[i].Tiles.Num());

		for (int j = 0; j < TileGrid[i].Tiles.Num(); j++) {
			UE_LOG(LogTemp, Warning, TEXT("Tile Spawn"));
			//Set up child actor
			UChildActorComponent* NewTile = NewObject<UChildActorComponent>(this, UChildActorComponent::StaticClass());
			NewTile->RegisterComponent();
			NewTile->SetChildActorClass(AActor_MapTile::StaticClass());
			NewTile->SetupAttachment(RootComponent);
			NewTile->SetRelativeLocation(FVector(i, j, TileGrid[i].Tiles[j].Height) * 100);

			//Set up tile
			TileGrid[i].Tiles[j].Tile = Cast<AActor_MapTile>(NewTile->GetChildActor());
			TileGrid[i].Tiles[j].Tile->SetCoordinates(this, i, j);
			TileGrid[i].Tiles[j].Tile->SetAtmosphere(EnvironmentType);
			TileGrid[i].Tiles[j].Tile->SetVoid(TileGrid[i].Tiles[j].bVoid);
		}
	}
}

void AGameMap::CreateGridArray()
{
	bSetUpInProgress = true;
	for (int i = 0; i < Rows; i++) {
		ColumnSetupArray.Empty();
		RowBeingSetUp = i;

		FRandomStream GenerationStream;
		GenerationStream.Initialize(WorkingSeed);

		for (int j = 0; j < Columns; j++) {
			FNeighbouringTileHeights HeightStruct = GetNeighbouringTileHeights(i, j);
			FTileDataC NewTileData;

			NewTileData.Tile = nullptr;
			float HeightMin = FMath::Clamp(HeightStruct.Shortest - MaxSlopeHeight, 0.0f, MaxTileHeight);
			float HeightMax = FMath::Clamp(HeightStruct.Tallest + MaxSlopeHeight, 0.0f, MaxTileHeight);
			NewTileData.Height = GenerationStream.FRandRange(HeightMin, HeightMax);
			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));

			float VoidRoll = GenerationStream.FRandRange(0.0f, 1.0f);
			if (VoidRoll < VoidWeight) {
				NewTileData.bVoid = true;
			}
			else {
				NewTileData.bVoid = false;
			}
			ColumnSetupArray.Add(NewTileData);
			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));
		}

		FTileRow NewRow;
		NewRow.Tiles = ColumnSetupArray;
		TileGrid.Add(NewRow);
	}
	bSetUpInProgress = false;
}

void AGameMap::PlaceMidpoint()
{
	float XCoord = (float)(Rows * 50) - 50;
	float YCoord = (float)(Columns * 50) - 50;
	Midpoint->SetRelativeLocation(FVector(XCoord, YCoord, 0.0f));
}

void AGameMap::DestroyMap()
{
	for (TActorIterator<AActor_MapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->Destroy();
	}

	for (TActorIterator<ABlockageC> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->Destroy();
	}

	TileGrid.Empty();
}

FNeighbouringTileHeights AGameMap::GetNeighbouringTileHeights(int TileX, int TileY)
{
	float CurrentTileHeight = 0.0f;
	float TotalHeight = 0.0f;
	float TallestTile = -1.0f;
	float ShortestTile = -1.0f;
	int TilesChecked = 0;

	FTileDataC TileNegX = GetTileDataAtCoordinates(TileX - 1, TileY);
	if (TileNegX.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TileNegX.Height;
		TotalHeight += TileNegX.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	FTileDataC TileNegY = GetTileDataAtCoordinates(TileX, TileY - 1);
	if (TileNegY.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TileNegY.Height;
		TotalHeight += TileNegY.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	FTileDataC TilePosX = GetTileDataAtCoordinates(TileX + 1, TileY);
	if (TilePosX.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TilePosX.Height;
		TotalHeight += TilePosX.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	FTileDataC TilePosY = GetTileDataAtCoordinates(TileX, TileY + 1);
	if (TilePosY.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TilePosY.Height;
		TotalHeight += TilePosY.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	FNeighbouringTileHeights ReturnData;
	if (TilesChecked > 0) {
		ReturnData.AverageHeight = TotalHeight / (float)TilesChecked;
		ReturnData.Tallest = TallestTile;
		ReturnData.Shortest = ShortestTile;
	}
	else {
		ReturnData.AverageHeight = FMath::FRandRange(0.0f, MaxTileHeight);
		ReturnData.Tallest = MaxTileHeight;
		ReturnData.Shortest = 0;
	}
	return ReturnData;
}

void AGameMap::ConsolidateHeightChanges(float& CurrentTileHeight, float& TallestTile, float& ShortestTile) {
	if (TallestTile < 0 || ShortestTile < 0) {
		TallestTile = CurrentTileHeight;
		ShortestTile = CurrentTileHeight;
	}
	else {
		if (CurrentTileHeight > TallestTile) {
			TallestTile = CurrentTileHeight;
		}
		else if (CurrentTileHeight > ShortestTile) {
			ShortestTile = CurrentTileHeight;
		}
	}
}

FTileDataC AGameMap::GetTileDataAtCoordinates(int TileX, int TileY)
{
	if (bSetUpInProgress && TileX == RowBeingSetUp) {
		if (ColumnSetupArray.Num() > TileY && TileY >= 0) {
			return ColumnSetupArray[TileY];
		}
	}
	else {
		if (TileGrid.Num() > TileX && TileX >= 0) {
			if (TileGrid[TileX].Tiles.Num() > TileY  && TileY >= 0) {
				return TileGrid[TileX].Tiles[TileY];
			}
		}
	}

	//Return a null representative if nothing returned yet
	FTileDataC NullData;
	NullData.Height = -1.0f; //Negative Height represents no data found
	return NullData;
}

bool AGameMap::DoesTileExist(int TileX, int TileY)
{
	if (bSetUpInProgress && RowBeingSetUp == TileX) {
		if (TileY >= 0 && TileY <= ColumnSetupArray.Num() - 1 && ColumnSetupArray.Num() > 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (TileX >= 0 && TileX <= TileGrid.Num() - 1 && TileGrid.Num() > 0) {
			if (TileY >= 0 && TileY <= TileGrid[TileX].Tiles.Num() - 1 && TileGrid[TileX].Tiles.Num() > 0) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
		}
	return true;
}

void AGameMap::SetAtmosphere(EEnvironmentEnum EnvironmentParam)
{
	EnvironmentType = EnvironmentParam;

	for (TActorIterator<AActor_MapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->SetAtmosphere(EnvironmentParam);
	}
}


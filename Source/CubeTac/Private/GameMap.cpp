// Copyright 2019 James Vigor. All Rights Reserved.


#include "GameMap.h"
#include "Blockage_TreeStump.h"
#include "Blockage_EnergyGeyser.h"

// Sets default values
AGameMap::AGameMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;
	
	//Initial Generation Settings
	WorkingSeed = 0;
	Rows = 10;
	Columns = 10;
	MaxTileHeight = 5.0f;
	MaxSlopeHeight = 0.5f;
	VoidWeight = 0.1f;
	BlockageWeight = 0.0f;
	GeyserWeight = 0.05f;
}

// Called when the game starts or when spawned
void AGameMap::BeginPlay()
{
	Super::BeginPlay();
	WorkingSeed = InitialSeed;
	GenerateMap();
}

// Runs the map building algorithm with existing settings. To define new settings, use the RegenerateMap function
void AGameMap::GenerateMap()
{
	if (this->HasAuthority()) {
		BuildMapGrid();

		// Determine whether each tile should have an energy geyser on it
		for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
		{
			FRandomStream GenerationStream;
			GenerationStream.Initialize(WorkingSeed);
			float GeyserRoll = GenerationStream.FRandRange(0.0f, 1.0f);
			if (GeyserRoll < GeyserWeight && !Itr->GetVoid()) {
				PlaceBlockageOnTile(*Itr, ABlockage_EnergyGeyser::StaticClass());
			}

			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));
		}

		// Determine whether each tile should have a blockage on it
		for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
		{
			FRandomStream GenerationStream;
			GenerationStream.Initialize(WorkingSeed);
			float BlockageRoll = GenerationStream.FRandRange(0.0f, 1.0f);
			if (BlockageRoll < BlockageWeight && !Itr->GetVoid() && Itr->GetBlockage() == nullptr) {
				PlaceBlockageOnTile(*Itr, ABlockage_TreeStump::StaticClass());
			}

			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));
		}

	}
}

// Runs the map building algorithm with new settings
// - Validation
bool AGameMap::RegenerateMap_Validate(int NewSeed, int RowsParam, int ColumnsParam, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam, float GeyserWeightParam)
{
	return true;
}

// - Implementation
void AGameMap::RegenerateMap_Implementation(int NewSeed, int RowsParam, int ColumnsParam, float TileHeightParam, float SlopeHeightParam, float VoidWeightParam, float BlockageWeightParam, float GeyserWeightParam)
{
	DestroyMap();  //Clear the space for a new map by removing the old one

	// Set parameters as settings
	WorkingSeed = FMath::Clamp(NewSeed, 0, 999999);
	Rows = RowsParam;
	Columns = ColumnsParam;
	MaxTileHeight = TileHeightParam;
	MaxSlopeHeight = SlopeHeightParam;
	VoidWeight = VoidWeightParam;
	BlockageWeight = BlockageWeightParam;
	GeyserWeight = GeyserWeightParam;
	GenerateMap();
}

// Generates tile data for the map and spawns the tiles accordingly
void AGameMap::BuildMapGrid()
{
	CreateGridArray();
	for (int i = 0; i < TileGrid.Num(); i++) {

		// Find location of map corner based on dimensions such that the map is always centred on the world origin point
		FVector OriginCorner = FVector((Rows * -50) + 50, (Columns * -50) + 50, 0.0f);

		for (int j = 0; j < TileGrid[i].Tiles.Num(); j++) {
			//Set up child actor
			UChildActorComponent* NewTile = NewObject<UChildActorComponent>(this, UChildActorComponent::StaticClass());
			NewTile->RegisterComponent();
			NewTile->SetChildActorClass(AMapTile::StaticClass());
			FAttachmentTransformRules TransformRules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, false);
			NewTile->AttachToComponent(RootComponent, TransformRules);
			NewTile->SetRelativeLocation(FVector(i, j, TileGrid[i].Tiles[j].Height) * 100 + OriginCorner);

			//Set up tile
			TileGrid[i].Tiles[j].Tile = Cast<AMapTile>(NewTile->GetChildActor());
			TileGrid[i].Tiles[j].Tile->SetCoordinates(this, i, j);
			TileGrid[i].Tiles[j].Tile->SetAtmosphere(EnvironmentType);
			TileGrid[i].Tiles[j].Tile->SetVoid(TileGrid[i].Tiles[j].bVoid);
		}
	}
}

// Generates tile data for BuildMapGrid function
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

			// Determine this tile's height
			float HeightMin = FMath::Clamp(HeightStruct.Shortest - MaxSlopeHeight, 0.0f, MaxTileHeight); //New tile can be no shorter than shortest neighbour - slope height
			float HeightMax = FMath::Clamp(HeightStruct.Tallest + MaxSlopeHeight, 0.0f, MaxTileHeight);  //New tile can be no taller than tallest neighbour + slope height
			NewTileData.Height = GenerationStream.FRandRange(HeightMin, HeightMax);

			// Regenerate seed
			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));

			// Determine whether this tile is void or not
			float VoidRoll = GenerationStream.FRandRange(0.0f, 1.0f);
			if (VoidRoll < VoidWeight) {
				NewTileData.bVoid = true;
			}
			else {
				NewTileData.bVoid = false;
			}
			ColumnSetupArray.Add(NewTileData);

			// Regenerate Seed
			WorkingSeed = FMath::FloorToInt(GenerationStream.FRandRange(0.0f, 999999.0f));
		}

		FTileRow NewRow;
		NewRow.Tiles = ColumnSetupArray;
		TileGrid.Add(NewRow);
	}
	bSetUpInProgress = false;
}

// Destroys all actors comprising the map and resets map data in order for the map to be generated again
void AGameMap::DestroyMap()
{
	// Destroy all map tiles
	for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->Destroy();
	}

	// Destroy all blockages
	for (TActorIterator<ABlockageC> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->Destroy();
	}

	// Clear tile data array
	TileGrid.Empty();
}


// Spawns a blockage actor and assigns it to the appropriate map tile
// - Validation
bool AGameMap::PlaceBlockageOnTile_Validate(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass)
{
	return true;
}

// - Implementation
void AGameMap::PlaceBlockageOnTile_Implementation(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass)
{
	FVector Location = Tile->GetActorLocation();
	FRotator Rotation = Tile->GetActorRotation();
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABlockageC* NewBlockage = GetWorld()->SpawnActor<ABlockageC>(BlockageClass, Location, Rotation, SpawnParameters);
	Tile->SetBlockage(NewBlockage);
}


// Returns a structure containing highest, lowest and average heights of any existing tiles orthogonally adjacent to the given coordinates
FNeighbouringTileHeights AGameMap::GetNeighbouringTileHeights(int TileX, int TileY)
{
	float CurrentTileHeight = 0.0f;
	float TotalHeight = 0.0f;
	float TallestTile = -1.0f;
	float ShortestTile = -1.0f;
	int TilesChecked = 0;

	// Check tile in negative X direction
	FTileDataC TileNegX = GetTileDataAtCoordinates(TileX - 1, TileY);
	if (TileNegX.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TileNegX.Height;
		TotalHeight += TileNegX.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	// Check tile in negative Y direction
	FTileDataC TileNegY = GetTileDataAtCoordinates(TileX, TileY - 1);
	if (TileNegY.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TileNegY.Height;
		TotalHeight += TileNegY.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	// Check tile in positive X direction
	FTileDataC TilePosX = GetTileDataAtCoordinates(TileX + 1, TileY);
	if (TilePosX.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TilePosX.Height;
		TotalHeight += TilePosX.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	// Check tile in positive Y direction
	FTileDataC TilePosY = GetTileDataAtCoordinates(TileX, TileY + 1);
	if (TilePosY.Height != -1.0f) { // Check valid data found
		CurrentTileHeight = TilePosY.Height;
		TotalHeight += TilePosY.Height;

		TilesChecked++;
	}
	ConsolidateHeightChanges(CurrentTileHeight, TallestTile, ShortestTile);

	FNeighbouringTileHeights ReturnData;
	if (TilesChecked > 0) {
		// Provite data for any tiles found
		ReturnData.AverageHeight = TotalHeight / (float)TilesChecked;
		ReturnData.Tallest = TallestTile;
		ReturnData.Shortest = ShortestTile;
	}
	else {
		// No tiles found (this should be the first tile to generate in the map) so generate initial values
		ReturnData.AverageHeight = MaxTileHeight / 2;
		ReturnData.Tallest = MaxTileHeight;
		ReturnData.Shortest = 0;
	}
	return ReturnData;
}

// Used by the GetNeighbouringTileHeights function to keep a track of the tallest and shortest tile heights found
void AGameMap::ConsolidateHeightChanges(float& CurrentTileHeight, float& TallestTile, float& ShortestTile) {
	if (TallestTile < 0 || ShortestTile < 0) {   // Values are less than 0 if no other tiles have been found
		// Make the current tile the standard to compare against
		TallestTile = CurrentTileHeight;
		ShortestTile = CurrentTileHeight;
	}
	else {   // Otherwise, see if current tile breaks any held records
		if (CurrentTileHeight > TallestTile) {
			TallestTile = CurrentTileHeight;
		}
		else if (CurrentTileHeight > ShortestTile) {
			ShortestTile = CurrentTileHeight;
		}
	}
}

// Returns a structure containing the data regarding the tile at the given coordinates
FTileDataC AGameMap::GetTileDataAtCoordinates(int TileX, int TileY)
{
	if (bSetUpInProgress && TileX == RowBeingSetUp) {   // Get data from ColumnSetupArray if the tile being referenced is still being set up
		if (ColumnSetupArray.Num() > TileY && TileY >= 0) {
			return ColumnSetupArray[TileY];
		}
	}
	else { // Otherwise get data from tile grid as data has been finalised
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

// Checks to see if a tile has been spawned at the given coordinates
bool AGameMap::DoesTileExist(int TileX, int TileY)
{
	if (bSetUpInProgress && RowBeingSetUp == TileX) {   // Get data from ColumnSetupArray if the tile being referenced is still being set up
		if (TileY >= 0 && TileY <= ColumnSetupArray.Num() - 1 && ColumnSetupArray.Num() > 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {   // Otherwise get data from tile grid as data has been finalised
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

// Set the appearance of the map tiles based on user preference
void AGameMap::SetAtmosphere(EEnvironmentEnum EnvironmentParam)
{
	EnvironmentType = EnvironmentParam;

	for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->SetAtmosphere(EnvironmentParam);
	}
}
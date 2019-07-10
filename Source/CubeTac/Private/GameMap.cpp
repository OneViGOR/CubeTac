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

}

void AGameMap::PlaceMidpoint()
{
	float XCoord = (float)(Rows * 50) - 50;
	float YCoord = (float)(Columns * 50) - 50;
	Midpoint->SetRelativeLocation(FVector(XCoord, YCoord, 0.0f));
}

void AGameMap::DestroyMap()
{

}

void AGameMap::SetAtmosphere(EEnvironmentEnum EnvironmentParam)
{
	EnvironmentType = EnvironmentParam;

	for (TActorIterator<AActor_MapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->SetAtmosphere(EnvironmentParam);
	}
}


// Copyright 2019 James Vigor. All Rights Reserved.


#include "MapTile.h"
#include "Engine.h"
#include "TileMarker.h"
#include "TacticalControllerBase.h"
#include "GridUnit.h"
#include "Unit_Portal.h"
#include "PlayerPawnC.h"
#include "TacticalGameState.h"
#include "UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
AMapTile::AMapTile()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	//Set Up Variables
	MovementCost = 1;

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Cap Mesh
	CapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CapMesh"));
	CapMesh->SetupAttachment(RootComponent);
	CapMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -5.0f));
	CapMesh->SetRelativeScale3D(FVector(1.1f, 1.1f, 0.1f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CapMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CapMeshAsset.Succeeded()) {
		CapMesh->SetStaticMesh(CapMeshAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CapMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Grass.Grass'"));
	if (CapMaterial.Succeeded()) {
		CapMesh->SetMaterial(0, CapMaterial.Object);
	}
	CapMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	//-Shaft Mesh
	ShaftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeamPlane"));
	ShaftMesh->SetupAttachment(RootComponent);
	ShaftMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -2010.0f));
	ShaftMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 40.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShaftMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (ShaftMeshAsset.Succeeded()) {
		ShaftMesh->SetStaticMesh(ShaftMeshAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShaftMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Dirt.Dirt'"));
	if (ShaftMaterial.Succeeded()) {
		ShaftMesh->SetMaterial(0, ShaftMaterial.Object);
	}
	ShaftMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	//-Tile Marker
	TileMarker = CreateDefaultSubobject<UChildActorComponent>(TEXT("TileMarker"));
	TileMarker->SetChildActorClass(ATileMarker::StaticClass());
	TileMarker->SetupAttachment(RootComponent);

	//Environment Data Table
	static ConstructorHelpers::FObjectFinder<UDataTable> EnvironmentDataAsset(TEXT("DataTable'/Game/Environment/Environment/EnvironmentColours.EnvironmentColours'"));

	if (EnvironmentDataAsset.Succeeded()) {
		EnvironmentData = EnvironmentDataAsset.Object;
	}

	//Set up mouse control events
	this->OnBeginCursorOver.AddDynamic(this, &AMapTile::OnBeginMouseOver);
	this->OnEndCursorOver.AddDynamic(this, &AMapTile::OnEndMouseOver);
	this->OnClicked.AddDynamic(this, &AMapTile::OnMouseClicked);
}

// Called when the game starts or when spawned
void AMapTile::BeginPlay()
{
	Super::BeginPlay();
	SetVoid(bVoid);
	SetHighlightMaterial();
}

// Sets up variable replication
void AMapTile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(AMapTile, OccupyingUnit);
	DOREPLIFETIME(AMapTile, GameMapReference);
	DOREPLIFETIME(AMapTile, Blockage);
	DOREPLIFETIME(AMapTile, bVoid);
}

// Set this tile up with a reference to the game map and coordinates within it
void AMapTile::SetCoordinates(AGameMap* Map, int X, int Y)
{
	GameMapReference = Map;
	Coordinates = FVector2D((float)X, (float)Y);
}


// Set this tile's void state. If true, its space in the map will appear and act unoccupied
// - Validation
bool AMapTile::SetVoid_Validate(bool bIsVoid)
{
	return true;
}

// - Implementation
void AMapTile::SetVoid_Implementation(bool bVoidParam)
{
	bVoid = bVoidParam;

	CapMesh->SetVisibility(!bVoid);
	ShaftMesh->SetVisibility(!bVoid);
	TileMarker->SetVisibility(!bVoid);

	// Disable visibility collisisons if void, otherwise enable them. Disabled visibility collisions result in no mouse input events
	if (bVoid) {
		CapMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		ShaftMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	}
	else {
		CapMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		ShaftMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}
}


// Set the appearance of this map tile based on user preference
// - Validation
bool AMapTile::SetAtmosphere_Validate(EEnvironmentEnum Environment)
{
	return true;
}

// - Implementation
void AMapTile::SetAtmosphere_Implementation(EEnvironmentEnum Environment)
{
	// Find data table row based on name of enumeration
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnvironmentEnum"), true);
	FString EnumString = EnumPtr->GetNameStringByIndex((int32)Environment);
	FName EnumName = FName(*EnumString.RightChop(4));
	static const FString ContextString(TEXT("GENERAL"));

	FEnvironment* FoundData = EnvironmentData->FindRow<FEnvironment>(EnumName, ContextString, true);

	// Set mesh materials to asset references from data table row
	CapMesh->SetMaterial(0, (UMaterialInterface*)FoundData->TileCapMaterial);
	ShaftMesh->SetMaterial(0, (UMaterialInterface*)FoundData->TileShaftMaterial);
	
}


// Assign a reference to the unit occupying this tile
// - Validation
bool AMapTile::SetOccupyingUnit_Validate(AGridUnit* NewOccupier)
{
	return true;
}

// - Implementation
void AMapTile::SetOccupyingUnit_Implementation(AGridUnit* NewOccupier)
{
	OccupyingUnit = NewOccupier;
}


// Cast a short line trace downwards to find a tile below the given point.Returns a reference to the tile, if found
AMapTile* AMapTile::LineTraceForTile(FVector Start)
{
	FHitResult Hit(ForceInit);
	FCollisionQueryParams CollisionParams;
	FVector StartPoint = Start + FVector(0.0f, 0.0f, 100.0f); // Raise the start point a small amount to provide a greater margin
	FVector EndPoint = Start + FVector(0.0f, 0.0f, -1000.0f);

	GetWorld()->LineTraceSingleByObjectType(Hit, StartPoint, EndPoint, ECC_WorldDynamic, CollisionParams); // Perform line trace

	// Return tile if found. If not, return nullptr
	if (Cast<AMapTile>(Hit.Actor)) {
		AMapTile* ReturnTile = Cast<AMapTile>(Hit.Actor);
		return ReturnTile;
	}
	else {
		return nullptr;
	}
}

// Returns up to four tiles orthogonally adjacent to this one
TArray<AMapTile*> AMapTile::GetFourNeighbouringTiles()
{
	TArray<AMapTile*> AllTilesFound;
	//Positive X direction
	AMapTile* TilePosX = LineTraceForTile(GetActorLocation() + FVector(100.0f, 0.0f, 0.0f));
	if (TilePosX != nullptr) {
		AllTilesFound.Add(TilePosX);
	}

	//Negative X direction
	AMapTile* TileNegX = LineTraceForTile(GetActorLocation() + FVector(-100.0f, 0.0f, 0.0f));
	if (TileNegX != nullptr) {
		AllTilesFound.Add(TileNegX);
	}

	//Positive Y direction
	AMapTile* TilePosY = LineTraceForTile(GetActorLocation() + FVector(0.0f, 100.0f, 0.0f));
	if (TilePosY != nullptr) {
		AllTilesFound.Add(TilePosY);
	}

	//Negative Y direction
	AMapTile* TileNegY = LineTraceForTile(GetActorLocation() + FVector(0.0f, -100.0f, 0.0f));
	if (TileNegY != nullptr) {
		AllTilesFound.Add(TileNegY);
	}

	return AllTilesFound;
}

// Update the colour and visibility of the tile marker based on known variables
void AMapTile::SetHighlightMaterial()
{
	if (!bVoid) {
		Cast<ATileMarker>(TileMarker->GetChildActor())->UpdateAppearance(this);
	}
}

// Contains behaviour for when the tile is clicked during the Gameplay phase
void AMapTile::ClickedInGamePhase()
{
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());

	// Move the selected unit if this tile is a target for movement
	if (ECurrentlyNavigable != ENavigationEnum::Nav_Unreachable) {
		Cast<APlayerPawnC>(PlayerController->GetPawn())->MoveUnit(this, PlayerController);
	}
	else {
		bool bSelectOccupiedCharacter = false;

		// Determine whether the player is trying to select a unit occupying this tile
		if (PlayerController->SelectedUnit != nullptr) {
			if (bTargetable) {
				if (Cast<AUnit_Portal>(PlayerController->SelectedUnit)) {  // If targeted by portal, portal is trying to spawn something
					AUnit_Portal* PortalReference = Cast<AUnit_Portal>(PlayerController->SelectedUnit);
					PortalReference->SpawnUnit(this);
				}
				else {  // If targeted by another unit, unit is trying to use an ability
					PlayerController->SelectedUnit->UseSelectedAbility(this);
				}
			}
			else {  // If neither targeted or navigable, player wishes to deselect a unit and may wish to select a different one instead
				PlayerController->SelectedUnit->DeselectUnit();
				bSelectOccupiedCharacter = true;
			}
		}
		else {
			bSelectOccupiedCharacter = true;  // If the player does not already have a unit selected, player wishes to select a new unit
		}

		// Ensure that a unit exists, is on the same team as the player and that the player wishes to select them before continuing
		if (OccupyingUnit != nullptr) {
			if (bSelectOccupiedCharacter && (PlayerController->Team == OccupyingUnit->GetTeam())) {
				OccupyingUnit->SelectUnit();
			}
		}
	}
}

// Contains behaviour for when the tile is clicked during the Portal Placement phase
void AMapTile::ClickedInPortalPlacementPhase()
{
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	APlayerPawnC* PlayerPawn = Cast<APlayerPawnC>(PlayerController->GetPawn());

	//A portal may be spawned as long at it is the player's turn and this tile has no blockage in the way
	if (PlayerController->bTurn && Blockage == nullptr) {
		if (PlayerController->OwnedPortal != nullptr) {
			PlayerController->OwnedPortal->GetCurrentTile()->SetOccupyingUnit(nullptr);
			PlayerPawn->DestroyActor(PlayerController->OwnedPortal);  // Players may only have one portal each. Remove any other that might exist
		}

		PlayerPawn->SpawnPortal(this, PlayerController->Team);
	}
}

// Deferred function that runs when a player cursor hovers over this tile
void AMapTile::OnBeginMouseOver(AActor* Actor) {
	if (Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() == EGamePhase::Phase_Portal || Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() == EGamePhase::Phase_Game) {
		bMouseOver = true;
		SetHighlightMaterial();
	}
}

// Deferred function that runs when a player cursor is no longer hovering over this tile
void AMapTile::OnEndMouseOver(AActor* Actor) {
	bMouseOver = false;
	SetHighlightMaterial();
}

// Deferred function that runs when a player clicks on this tile
void AMapTile::OnMouseClicked(AActor* Actor, FKey ButtonPressed) {
	if (!bVoid) {

		// Run the correct code based on the current game phase
		if (GWorld->GetGameState() == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("No gamestate found!"));
		}
		else {
			if (Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() == EGamePhase::Phase_Portal) {
				ClickedInPortalPlacementPhase();
			}
			else if (Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() == EGamePhase::Phase_Game) {
				ClickedInGamePhase();
			}
		}
	}
}


// Getters and Setters

// Returns the movement energy it takes to leave this tile, excluding that added by any blockage on this tile
int AMapTile::GetBaseMovementCost()
{
	return MovementCost;
}

// Returns the total movement energy it takes to leave this tile, including that added by any blockage on this tile
int AMapTile::GetTotalMovementCost()
{
	int TotalCost = MovementCost;
	if (Blockage != nullptr) {
		TotalCost += Blockage->AdditionalMovementCost; // Add additional cost from blockage if one is present
	}

	return TotalCost;
}

// Returns a pointer to the unit currently occupying this tile, if there is one
AGridUnit* AMapTile::GetOccupyingUnit()
{
	return OccupyingUnit;
}

// Sets this tile's reference to its new assigned blockage
void AMapTile::SetBlockage(ABlockageC * NewBlockage)
{
	Blockage = NewBlockage;
}

// Returns a pointer to the blockage currently on this tile, if there is one
ABlockageC* AMapTile::GetBlockage()
{
	return Blockage;
}

// Returns true if this tile is considered void. Otherwise, returns false
bool AMapTile::GetVoid()
{
	return bVoid;
}
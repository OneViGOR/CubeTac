// Fill out your copyright notice in the Description page of Project Settings.


#include "MapTile.h"
#include "Engine.h"
#include "TileMarker.h"
#include "TacticalControllerBase.h"
#include "GridCharacterC.h"
#include "PortalC.h"
#include "PlayerPawnC.h"
#include "TacticalGameState.h"
#include "UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
AMapTile::AMapTile()
{
	PrimaryActorTick.bCanEverTick = false;

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

void AMapTile::BeginPlay()
{
	Super::BeginPlay();
	SetVoid(bVoid);
	SetHighlightMaterial();
}

void AMapTile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(AMapTile, OccupyingCharacter);
	DOREPLIFETIME(AMapTile, GameMapReference);
	DOREPLIFETIME(AMapTile, Blockage);
	DOREPLIFETIME(AMapTile, bVoid);
}

void AMapTile::SetCoordinates(AGameMap* Map, int X, int Y)
{
	GameMapReference = Map;
	Coordinates = FVector2D((float)X, (float)Y);
}

bool AMapTile::SetVoid_Validate(bool bIsVoid)
{
	return true;
}

void AMapTile::SetVoid_Implementation(bool bVoidParam)
{
	bVoid = bVoidParam;

	CapMesh->SetVisibility(!bVoid);
	ShaftMesh->SetVisibility(!bVoid);
	TileMarker->SetVisibility(!bVoid);

	if (bVoid) {
		CapMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		ShaftMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	}
	else {
		CapMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		ShaftMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}
}

bool AMapTile::SetAtmosphere_Validate(EEnvironmentEnum Environment)
{
	return true;
}

void AMapTile::SetAtmosphere_Implementation(EEnvironmentEnum Environment)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnvironmentEnum"), true);
	FString enumString = EnumPtr->GetNameStringByIndex((int32)Environment);
	FName enumName = FName(*enumString.RightChop(4));
	static const FString ContextString(TEXT("GENERAL"));

	FEnvironment* foundData = EnvironmentData->FindRow<FEnvironment>(enumName, ContextString, true);

	CapMesh->SetMaterial(0, (UMaterialInterface*)foundData->tileCapMaterial);
	ShaftMesh->SetMaterial(0, (UMaterialInterface*)foundData->tileShaftMaterial);
	
}

bool AMapTile::SetOccupyingCharacter_Validate(AGridCharacterC* NewOccupier)
{
	return true;
}

void AMapTile::SetOccupyingCharacter_Implementation(AGridCharacterC* NewOccupier)
{
	OccupyingCharacter = NewOccupier;
}

AMapTile* AMapTile::LineTraceForTile(FVector Start)
{
	FHitResult Hit(ForceInit);
	FCollisionQueryParams CollisionParams;
	FVector StartPoint = Start + FVector(0.0f, 0.0f, 100.0f);
	FVector EndPoint = Start + FVector(0.0f, 0.0f, -1000.0f);

	GetWorld()->LineTraceSingleByObjectType(Hit, StartPoint, EndPoint, ECC_WorldDynamic, CollisionParams);

	if (Cast<AMapTile>(Hit.Actor)) {
		AMapTile* ReturnTile = Cast<AMapTile>(Hit.Actor);
		return ReturnTile;
	}
	else {
		return nullptr;
	}
}

int AMapTile::GetTotalMovementCost()
{
	int TotalCost = MovementCost;
	if (Blockage->IsValidLowLevel()) {
		TotalCost += Blockage->AdditionalMovementCost;
	}
	return TotalCost;
}

TArray<AMapTile*> AMapTile::GetFourNeighbouringTiles()
{
	TArray<AMapTile*> Tiles;
	return Tiles;
}

void AMapTile::SetHighlightMaterial()
{
	if (!bVoid) {
		Cast<ATileMarker>(TileMarker->GetChildActor())->UpdateAppearance(this);
	}
}

// Mouse Control
void AMapTile::ClickedInGamePhase()
{
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());

	if (ECurrentlyNavigable != ENavigationEnum::Nav_Unreachable) {
		Cast<APlayerPawnC>(PlayerController->GetPawn())->MoveCharacter(this, PlayerController);
	}
	else {
		bool bSelectOccupiedCharacter = false;

		if (PlayerController->SelectedCharacter->IsValidLowLevel()) {
			if (bTargetable) {
				if (Cast<APortalC>(PlayerController->SelectedCharacter)) {
					APortalC* PortalReference = Cast<APortalC>(PlayerController->SelectedCharacter);
					PortalReference->SpawnCharacter(this);
				}
				else {
					PlayerController->SelectedCharacter->UseSelectedAbility(this);
				}
			}
			else {
				PlayerController->SelectedCharacter->DeselectCharacter();
				bSelectOccupiedCharacter = true;
			}
		}
		else {
			bSelectOccupiedCharacter = true;
		}

		if (bSelectOccupiedCharacter && (PlayerController->Team == OccupyingCharacter->GetTeam())) {
			OccupyingCharacter->SelectCharacter();
		}
	}
}

void AMapTile::ClickedInPortalPlacementPhase()
{
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	APlayerPawnC* PlayerPawn = Cast<APlayerPawnC>(PlayerController->GetPawn());

	if (PlayerController->bTurn && !(Blockage->IsValidLowLevel())) {
		if (PlayerController->OwnedPortal->IsValidLowLevel()) {
			PlayerPawn->DestroyActor(PlayerController->OwnedPortal);
		}

		UE_LOG(LogTemp, Warning, TEXT("Should execute %s"), PlayerPawn->IsValidLowLevel() ? TEXT("True") : TEXT("False"));
		PlayerPawn->SpawnPortal(this, PlayerController->Team);
	}
}

void AMapTile::OnBeginMouseOver(AActor* Component) {
	if (Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() != EGamePhase::Phase_Lobby) {
		bMouseOver = true;
		SetHighlightMaterial();
	}
}

void AMapTile::OnEndMouseOver(AActor* Component) {
	bMouseOver = false;
	SetHighlightMaterial();
}

void AMapTile::OnMouseClicked(AActor* Component, FKey ButtonPressed) {
	if (!bVoid) {
		if (Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() == EGamePhase::Phase_Portal) {
			ClickedInPortalPlacementPhase();
		}
		else if (Cast<ATacticalGameState>(GWorld->GetGameState())->GetGamePhase() == EGamePhase::Phase_Game) {
			ClickedInGamePhase();
		}
	}
}

// Getters and Setters
AGridCharacterC* AMapTile::GetOccupyingCharacter()
{
	return OccupyingCharacter;
}

ABlockageC* AMapTile::GetBlockage()
{
	return Blockage;
}

bool AMapTile::GetVoid()
{
	return bVoid;
}
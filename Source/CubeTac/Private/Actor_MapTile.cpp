// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor_MapTile.h"
#include "Engine.h"
#include "TileMarker.h"
#include "TacticalControllerC.h"
#include "GridCharacterC.h"
#include "PortalC.h"
#include "PlayerPawnC.h"
#include "TacticalGameState.h"
#include "UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
AActor_MapTile::AActor_MapTile()
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
	this->OnBeginCursorOver.AddDynamic(this, &AActor_MapTile::OnBeginMouseOver);
	this->OnEndCursorOver.AddDynamic(this, &AActor_MapTile::OnEndMouseOver);
	this->OnClicked.AddDynamic(this, &AActor_MapTile::OnMouseClicked);
}

void AActor_MapTile::BeginPlay()
{
	Super::BeginPlay();
	SetVoid(bVoid);
	SetHighlightMaterial();
}

void AActor_MapTile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(AActor_MapTile, OccupyingCharacter);
	DOREPLIFETIME(AActor_MapTile, GameMapReference);
	DOREPLIFETIME(AActor_MapTile, Blockage);
	DOREPLIFETIME(AActor_MapTile, bVoid);
}

void AActor_MapTile::SetCoordinates(AGameMap* Map, int X, int Y)
{
	GameMapReference = Map;
	Coordinates = FVector2D((float)X, (float)Y);
}

bool AActor_MapTile::SetVoid_Validate(bool bIsVoid)
{
	return true;
}

void AActor_MapTile::SetVoid_Implementation(bool bVoidParam)
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

bool AActor_MapTile::SetAtmosphere_Validate(EEnvironmentEnum Environment)
{
	return true;
}

void AActor_MapTile::SetAtmosphere_Implementation(EEnvironmentEnum Environment)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnvironmentEnum"), true);
	FString enumString = EnumPtr->GetNameStringByIndex((int32)Environment);
	FName enumName = FName(*enumString.RightChop(4));
	static const FString ContextString(TEXT("GENERAL"));

	FEnvironment* foundData = EnvironmentData->FindRow<FEnvironment>(enumName, ContextString, true);

	CapMesh->SetMaterial(0, (UMaterialInterface*)foundData->tileCapMaterial);
	ShaftMesh->SetMaterial(0, (UMaterialInterface*)foundData->tileShaftMaterial);
	
}

bool AActor_MapTile::SetOccupyingCharacter_Validate(AGridCharacterC* NewOccupier)
{
	return true;
}

void AActor_MapTile::SetOccupyingCharacter_Implementation(AGridCharacterC* NewOccupier)
{
	OccupyingCharacter = NewOccupier;
}

AActor_MapTile* AActor_MapTile::LineTraceForTile(FVector Start)
{
	FHitResult Hit(ForceInit);
	FCollisionQueryParams CollisionParams;
	FVector StartPoint = Start + FVector(0.0f, 0.0f, 100.0f);
	FVector EndPoint = Start + FVector(0.0f, 0.0f, -1000.0f);

	GetWorld()->LineTraceSingleByObjectType(Hit, StartPoint, EndPoint, ECC_WorldDynamic, CollisionParams);

	if (Cast<AActor_MapTile>(Hit.Actor)) {
		AActor_MapTile* ReturnTile = Cast<AActor_MapTile>(Hit.Actor);
		return ReturnTile;
	}
	else {
		return nullptr;
	}
}

int AActor_MapTile::GetTotalMovementCost()
{
	int TotalCost = MovementCost;
	if (Blockage->IsValidLowLevel()) {
		TotalCost += Blockage->AdditionalMovementCost;
	}
	return TotalCost;
}

TArray<AActor_MapTile*> AActor_MapTile::GetFourNeighbouringTiles()
{
	TArray<AActor_MapTile*> Tiles;
	return Tiles;
}

void AActor_MapTile::SetHighlightMaterial()
{
	if (!bVoid) {
		Cast<ATileMarker>(TileMarker->GetChildActor())->UpdateAppearance(this);
	}
}

// Mouse Control
void AActor_MapTile::ClickedInGamePhase()
{
	ATacticalControllerC* PlayerController = Cast<ATacticalControllerC>(GetWorld()->GetFirstPlayerController());

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

void AActor_MapTile::ClickedInPortalPlacementPhase()
{
	ATacticalControllerC* PlayerController = Cast<ATacticalControllerC>(GetWorld()->GetFirstPlayerController());
	APlayerPawnC* PlayerPawn = Cast<APlayerPawnC>(PlayerController->GetPawn());

	if (PlayerController->bTurn && !(Blockage->IsValidLowLevel())) {
		if (PlayerController->OwnedPortal->IsValidLowLevel()) {
			PlayerPawn->DestroyActor(PlayerController->OwnedPortal);
		}
		PlayerPawn->SpawnPortal(this, PlayerController->Team);
	}
}

void AActor_MapTile::OnBeginMouseOver(AActor* Component) {
	if (Cast<ATacticalGameState>(GWorld->GetGameState())->GamePhase != EGamePhase::Phase_Lobby) {
		bMouseOver = true;
		SetHighlightMaterial();
	}
}

void AActor_MapTile::OnEndMouseOver(AActor* Component) {
	bMouseOver = false;
	SetHighlightMaterial();
}

void AActor_MapTile::OnMouseClicked(AActor* Component, FKey ButtonPressed) {
	if (!bVoid) {
		if (Cast<ATacticalGameState>(GWorld->GetGameState())->GamePhase == EGamePhase::Phase_Portal) {
			ClickedInPortalPlacementPhase();
		}
		else if (Cast<ATacticalGameState>(GWorld->GetGameState())->GamePhase == EGamePhase::Phase_Game) {
			ClickedInGamePhase();
		}
	}
}

// Getters and Setters
AGridCharacterC* AActor_MapTile::GetOccupyingCharacter()
{
	return OccupyingCharacter;
}

ABlockageC* AActor_MapTile::GetBlockage()
{
	return Blockage;
}

bool AActor_MapTile::GetVoid()
{
	return bVoid;
}
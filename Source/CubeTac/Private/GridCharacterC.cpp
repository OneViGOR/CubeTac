// Fill out your copyright notice in the Description page of Project Settings.


#include "GridCharacterC.h"
#include "PortalC.h"
#include "TacticalControllerBase.h"
#include "UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

UFUNCTION(Server)
template <class T>
T* PlaceBlockageOnTile(AMapTile* Tile)
{
	Tile->Blockage = GetWorld()->SpawnActor<T>(T::StaticClass(), NAME_None, Tile->GetActorLocation(), Tile->GetActorRotation(), NULL, false, false, Owner, Instigator);
	return actor;
}

// Sets default values
AGridCharacterC::AGridCharacterC()
{
	PrimaryActorTick.bCanEverTick = false;

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Character Mesh
	CharacterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CharacterMeshAsset(TEXT("/Game/Geometry/SkeletalMeshes/Human.Human"));
	if (CharacterMeshAsset.Succeeded()) {
		CharacterMesh->SetStaticMesh(CharacterMeshAsset.Object);
		
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterial(TEXT("/Game/Materials/PlainColours/Character.Character"));
	if (CharacterMaterial.Succeeded()) {
		CharacterMesh->SetMaterial(0, CharacterMaterial.Object);
	}

	//-Team Colour Plane
	TeamPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeamPlane"));
	TeamPlane->SetupAttachment(RootComponent);
	TeamPlane->SetRelativeLocation(FVector(0.0f, 0.0f, 1.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TeamPlaneAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (TeamPlaneAsset.Succeeded()) {
		TeamPlane->SetStaticMesh(TeamPlaneAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TeamPlaneMaterial(TEXT("/Game/Materials/TeamLight.TeamLight"));
	if (TeamPlaneMaterial.Succeeded()){
		TeamPlane->SetMaterial(0, TeamPlaneMaterial.Object);
	}

	//Timeline
	const ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("CurveFloat'/Game/GameData/Curves/1Sec_1to0.1Sec_1to0'"));
	SpawnDissolveTimeline = FTimeline{};
	FOnTimelineFloat progressFunction{};
	progressFunction.BindUFunction(this, "DissolveTick"); // The function DissolveTick gets called
	SpawnDissolveTimeline.AddInterpFloat(Curve.Object, progressFunction, FName{ TEXT("SpawnDissolve") });

	//Particle Systems
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/CharacterDeath.CharacterDeath'"));
	if (ParticleSystemClass.Succeeded()) {
		DeathParticles = Cast<UParticleSystem>(ParticleSystemClass.Object);
	}
}

void AGridCharacterC::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(AGridCharacterC, Health);
	DOREPLIFETIME(AGridCharacterC, MovesRemaining);
	DOREPLIFETIME(AGridCharacterC, Energy);
	DOREPLIFETIME(AGridCharacterC, MaxEnergy);
	DOREPLIFETIME(AGridCharacterC, CurrentTile);
	DOREPLIFETIME(AGridCharacterC, Team);
}

// Called when the game starts or when spawned
void AGridCharacterC::BeginPlay()
{
	Super::BeginPlay();
	DetermineCurrentTile();
	SetTeamLightColour();
	SpawnDissolve();

	SpawnDissolveTimeline.PlayFromStart();
}

void AGridCharacterC::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

// Called to bind functionality to input
void AGridCharacterC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Health
void AGridCharacterC::Death()
{
	DeselectCharacter();
	CurrentTile->SetOccupyingCharacter(nullptr);
	SpawnParticleEffectMulticast(DeathParticles, this->GetActorLocation());
}

float AGridCharacterC::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	Health = FMath::Clamp(Health - FMath::TruncToInt(DamageAmount), 0, MaxHealth);
	if (Health == 0) {
		Death();
	}
	return Health;
}


bool AGridCharacterC::DamageTarget_Validate(AActor* DamagedActor, float Damage)
{
	return true;
}

void AGridCharacterC::DamageTarget_Implementation(AActor* DamagedActor, float Damage)
{
	TSubclassOf<UDamageType> DamageType;
	UGameplayStatics::ApplyDamage(DamagedActor, Damage, nullptr, nullptr, DamageType);
}


// Selection
void AGridCharacterC::SelectCharacter()
{
	bSelected = true;
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	PlayerController->CharacterSelected(this);
	ShowNavigableLocations(CurrentTile);
	CancelTargetting();
}

void AGridCharacterC::DeselectCharacter()
{
	bSelected = false;
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	PlayerController->CharacterSelected(nullptr);
	CancelAllNavigableLocations();
	CancelTargetting();
}

// Navigation
ENavigationEnum AGridCharacterC::CanReachTile(AMapTile* Destination) //NEEDS LOGIC
{
	return ENavigationEnum::Nav_Safe;
}

bool AGridCharacterC::ShowNavigableLocations_Validate(AMapTile* FromTile)
{
	return true;
}

void AGridCharacterC::ShowNavigableLocations_Implementation(AMapTile* FromTile)
{
	CancelAllNavigableLocations();
	int MovesRequired;
	if (bIgnoresBlockageSlowing) {
		MovesRequired = 1;
	} else {
		MovesRequired = CurrentTile->GetTotalMovementCost();
	}
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	if (MovesRequired >= MovesRemaining && PlayerController->bTurn) {
		TArray<AMapTile*> TilesInRange = FromTile->GetFourNeighbouringTiles();
		for (int i = 0; i < TilesInRange.Num(); i++) {
			AMapTile* Tile = TilesInRange[i];
			Tile->ECurrentlyNavigable = CanReachTile(Tile);
			Tile->SetHighlightMaterial();
		}
	}
}

void AGridCharacterC::CancelAllNavigableLocations()
{
	for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->ECurrentlyNavigable = ENavigationEnum::Nav_Unreachable;
		Itr->SetHighlightMaterial();
	}
}

bool AGridCharacterC::CheckTileForProperties(AMapTile* Tile, bool bCheckSelf, bool bCheckEnemies, bool bCheckAllies, bool bCheckEmptyTiles, bool bCheckAlliedPortal, bool bCheckEnemyPortal, bool bCheckBlockages)
{
	//Check for Self
	bool bSelfFound = false;
	if (bCheckSelf) {
		bSelfFound = (CurrentTile == Tile);
	}
	else {
		bSelfFound = true; //Always allow success here if not searching for self
	}

	//Check for Enemies
	bool bEnemyFound = false;
	if (bCheckEnemies) {
		if ((CurrentTile != Tile) && (Tile->GetOccupyingCharacter()->IsValidLowLevel())) {
			bEnemyFound = ((Tile->GetOccupyingCharacter()->Team != Team) && !(Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}
	else {
		bEnemyFound = true; //Always allow success here if not searching for enemies
	}

	//Check for Allies
	bool bAllyFound = false;
	if (bCheckAllies) {
		if ((CurrentTile != Tile) && (Tile->GetOccupyingCharacter()->IsValidLowLevel())) {
			bAllyFound = ((Tile->GetOccupyingCharacter()->Team == Team) && !(Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}
	else {
		bAllyFound = true; //Always allow success here if not searching for allies
	}

	//Check for Empty Tile
	bool bEmptyFound = false;
	if (bCheckEmptyTiles) {
		bEmptyFound = (!(Tile->GetOccupyingCharacter()->IsValidLowLevel()) && !(Tile->GetBlockage()->IsValidLowLevel()));
	}
	else {
		bEmptyFound = true; //Always allow success here if not searching for empty tiles
	}

	//Check for Allied Portal
	bool bAlliedPortalFound = false;
	if (bCheckAlliedPortal) {
		if (Tile->GetOccupyingCharacter()->IsValidLowLevel()) {
			bAlliedPortalFound = ((Tile->GetOccupyingCharacter()->Team == Team) && (Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}
	else {
		bAlliedPortalFound = true; //Always allow success here if not searching for allied portals
	}
	

	//Check for Enemy Portal
	bool bEnemyPortalFound = false;
	if (bCheckEnemyPortal) {
		if (Tile->GetOccupyingCharacter()->IsValidLowLevel()) {
			bEnemyPortalFound = ((Tile->GetOccupyingCharacter()->Team != Team) && (Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}
	else {
		bEnemyPortalFound = true; //Always allow success here if not searching for enemy portals
	}

	//Check for Blockage
	bool bBlockageFound = false;
	if (bCheckEmptyTiles) {
		bBlockageFound = (Tile->GetBlockage()->IsValidLowLevel());
	}
	else {
		bBlockageFound = true; //Always allow success here if not searching for blockages
	}

	return (bSelfFound && bEnemyFound && bAllyFound && bEmptyFound && bAlliedPortalFound && bEnemyPortalFound && bBlockageFound);
}

void AGridCharacterC::DetermineCurrentTile() {
	FHitResult Hit(ForceInit);
	FCollisionQueryParams CollisionParams;
	FVector StartPoint = this->GetActorLocation();
	FVector EndPoint = StartPoint + FVector(0.0f, 0.0f, -500.0f);

	GetWorld()->LineTraceSingleByObjectType(Hit, StartPoint, EndPoint, ECC_WorldDynamic, CollisionParams);

	if (Cast<AMapTile>(Hit.Actor)) {
		CurrentTile = Cast<AMapTile>(Hit.Actor);
		CurrentTile->SetOccupyingCharacter(this);
		SetActorLocation(CurrentTile->GetActorLocation());
	}
}

// Abilities
void AGridCharacterC::SelectAbility(int Ability)
{
	CancelAllNavigableLocations();
	CancelTargetting();
	SelectedAbility = Ability;
	FCharacterAbility SelectedAbilityStructure = AbilitySet[Ability - 1];
	TArray<AMapTile*> TilesInRange = FindAbilityRange(SelectedAbilityStructure.Range);

	for (int i = 0; i < TilesInRange.Num(); i++)
	{
		bool bAbilityRulePass = false;
		switch (SelectedAbility) {
		case 1:
			bAbilityRulePass = AbilityRule1();
		case 2:
			bAbilityRulePass = AbilityRule2();
		case 3:
			bAbilityRulePass = AbilityRule3();
		}

		if (bAbilityRulePass && CheckTileForProperties(TilesInRange[i], SelectedAbilityStructure.bAffectsSelf, SelectedAbilityStructure.bAffectsEnemies, SelectedAbilityStructure.bAffectsAllies, SelectedAbilityStructure.bAffectsTiles, SelectedAbilityStructure.bAffectsAlliedPortal, SelectedAbilityStructure.bAffectsEnemyPortal, SelectedAbilityStructure.bAffectsBlockages)) {
			TilesInRange[i]->bTargetable = true;
			TilesInRange[i]->SetHighlightMaterial();
		}
	}
}

TArray<AMapTile*> AGridCharacterC::FindAbilityRange(int Range)
{
	TArray<AMapTile*> TilesInRange;
	TilesInRange.Add(CurrentTile);
	TilesInRange = ExpandAbilityRange(TilesInRange, Range);
	return TilesInRange;
}

TArray<AMapTile*> AGridCharacterC::ExpandAbilityRange(TArray<AMapTile*> CurrentRange, int Range)
{
	TArray<AMapTile*> TilesToAdd;
	for (int i = 0; i < CurrentRange.Num(); i++) {
		TArray<AMapTile*> Neighbours = CurrentRange[i]->GetFourNeighbouringTiles();
		for (int j = 0; j < Neighbours.Num(); j++) {
			TilesToAdd.AddUnique(Neighbours[j]);
		}
	}
	
	if (Range - 1 > 0) {
		TArray<AMapTile*> FurtherExpansion = ExpandAbilityRange(TilesToAdd, Range - 1);
		for (int i = 0; i < FurtherExpansion.Num(); i++) {
			TilesToAdd.AddUnique(FurtherExpansion[i]);
		}
	}

	for (int i = 0; i < CurrentRange.Num(); i++) {
		TilesToAdd.AddUnique(CurrentRange[i]);
	}

	return TilesToAdd;
}

void AGridCharacterC::CancelTargetting()
{
	SelectedAbility = 0;
	for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->bTargetable = false;
		Itr->SetHighlightMaterial();
	}
}

bool AGridCharacterC::DestroyBlockageOnTile_Validate(AMapTile* Tile) {
	return true;
}

void AGridCharacterC::DestroyBlockageOnTile_Implementation(AMapTile* Tile) {
	Tile->GetBlockage()->Destroy();
}

bool AGridCharacterC::UseSelectedAbility_Validate(AMapTile* TargetTile)
{
	return true;
}

void AGridCharacterC::UseSelectedAbility_Implementation(AMapTile* TargetTile)
{
	switch (SelectedAbility) {
	case 1:
		Ability1(TargetTile);
	case 2:
		Ability2(TargetTile);
	case 3:
		Ability3(TargetTile);
	}

	Energy = FMath::Clamp(Energy - (AbilitySet[SelectedAbility - 1].Cost), 0, MaxEnergy);
	CancelTargetting();
}

void AGridCharacterC::Ability1(AMapTile* TargetTile)
{

}

void AGridCharacterC::Ability2(AMapTile* TargetTile)
{

}

void AGridCharacterC::Ability3(AMapTile* TargetTile)
{

}

bool AGridCharacterC::AbilityRule1()
{
	return true;
}

bool AGridCharacterC::AbilityRule2()
{
	return true;
}

bool AGridCharacterC::AbilityRule3()
{
	return true;
}

// Turn Management
bool AGridCharacterC::NewTurnStart_Validate()
{
	return true;
}

void AGridCharacterC::NewTurnStart_Implementation()
{
	ResetMovesAndEnergy();
}

bool AGridCharacterC::ResetMovesAndEnergy_Validate()
{
	return true;
}

void AGridCharacterC::ResetMovesAndEnergy_Implementation()
{
	MovesRemaining = TurnMoveDistance;
	Energy = MaxEnergy;
}

// Cosmetic
bool AGridCharacterC::CallForParticles_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

void AGridCharacterC::CallForParticles_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	SpawnParticleEffectServer_Implementation(EmitterTemplate, Location);
}

bool AGridCharacterC::SpawnParticleEffectServer_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

void AGridCharacterC::SpawnParticleEffectServer_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	SpawnParticleEffectMulticast_Implementation(EmitterTemplate, Location);
}

bool AGridCharacterC::SpawnParticleEffectMulticast_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

void AGridCharacterC::SpawnParticleEffectMulticast_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplate, Location, FRotator::ZeroRotator, true);
}

void AGridCharacterC::SetTeamLightColour()
{

}

void AGridCharacterC::SpawnDissolve()
{

}

void AGridCharacterC::DissolveTick(float Value)
{

}

// Getters and Setters
void AGridCharacterC::SetSelected(bool bNewValue) {
	bSelected = bNewValue;
}

bool AGridCharacterC::GetSelected() {
	return bSelected;
}

int AGridCharacterC::GetTeam() {
	return Team;
}
// Copyright 2019 James Vigor. All Rights Reserved.


#include "GridUnit.h"
#include "Unit_Portal.h"
#include "TacticalControllerBase.h"
#include "TacticalGameState.h"
#include "PlayerPawnC.h"
#include "UnrealNetwork.h"
#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"


// Sets default values
AGridUnit::AGridUnit()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Unit Mesh
	UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
	UnitMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("/Game/Geometry/SkeletalMeshes/Human.Human"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
		
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitSpawnMaterial(TEXT("/Game/Materials/PlainColours/Character.Character"));
	if (UnitSpawnMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitSpawnMaterial.Object);
		SpawnDissolveMaterial = UnitSpawnMaterial.Object;
	}
	UnitMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitDeathMaterial(TEXT("/Game/Materials/PlainColours/Character.Character"));
	if (UnitDeathMaterial.Succeeded()) {
		DeathDissolveMaterial = UnitDeathMaterial.Object;
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
	TeamPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn Timeline
	SpawnDissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SpawnDissolveTimeline"));
	const ConstructorHelpers::FObjectFinder<UCurveFloat> SpawnCurveAsset(TEXT("CurveFloat'/Game/GameData/Curves/1Sec_1to0.1Sec_1to0'"));
	if (SpawnCurveAsset.Succeeded()) {
		SpawnCurve = SpawnCurveAsset.Object;
	}
	FOnTimelineFloat SpawnProgressFunction{};
	SpawnProgressFunction.BindUFunction(this, FName("DissolveTick")); // The function DissolveTick gets called when the timeline updates
	SpawnDissolveTimeline->AddInterpFloat(SpawnCurve, SpawnProgressFunction, FName("Alpha"));

	//Death Timeline
	DeathDissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DeathDissolveTimeline"));
	const ConstructorHelpers::FObjectFinder<UCurveFloat> DeathCurveAsset(TEXT("CurveFloat'/Game/GameData/Curves/3Sec_0to1.3Sec_0to1'"));
	if (DeathCurveAsset.Succeeded()) {
		DeathCurve = DeathCurveAsset.Object;
	}
	FOnTimelineFloat DeathProgressFunction{};
	FOnTimelineEventStatic DeathFinishedFunction;
	DeathProgressFunction.BindUFunction(this, FName("DissolveTick")); // The function DissolveTick gets called when the timeline updates
	DeathFinishedFunction.BindUFunction(this, FName("DeathAnimationComplete"));
	DeathDissolveTimeline->AddInterpFloat(DeathCurve, DeathProgressFunction, FName("Alpha"));
	DeathDissolveTimeline->SetTimelineFinishedFunc(DeathFinishedFunction);

	//Particle Systems
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/CharacterDeath.CharacterDeath'"));
	if (ParticleSystemClass.Succeeded()) {
		DeathParticles = Cast<UParticleSystem>(ParticleSystemClass.Object);
	}

}

// Sets up variable replication
void AGridUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate to all
	DOREPLIFETIME(AGridUnit, Health);
	DOREPLIFETIME(AGridUnit, MovesRemaining);
	DOREPLIFETIME(AGridUnit, Energy);
	DOREPLIFETIME(AGridUnit, MaxEnergy);
	DOREPLIFETIME(AGridUnit, CurrentTile);
	DOREPLIFETIME(AGridUnit, Team);
}

// Called when the game starts or when spawned
void AGridUnit::BeginPlay()
{
	Super::BeginPlay();
	DetermineCurrentTile();
	SetTeamLightColour();

	// Make a dynamic material from the mesh default material
	DynMaterial = UnitMesh->CreateDynamicMaterialInstance(0, UnitMesh->GetMaterial(0));
	SpawnDissolveTimeline->PlayFromStart();
}

// Called to bind functionality to input
void AGridUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Handles the death of a unit
void AGridUnit::Death()
{
	// Clear references to this unit
	DeselectUnit();
	CurrentTile->SetOccupyingUnit(nullptr);
	SpawnParticleEffectMulticast(DeathParticles, this->GetActorLocation());

	// Run death dissolve animation
	DynMaterial = UnitMesh->CreateDynamicMaterialInstance(0, DeathDissolveMaterial);
	DeathDissolveTimeline->PlayFromStart();
}

// OVERRIDE - Called when damage is inflicted upon this unit
float AGridUnit::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	//Subtract damage from health to a minimum of 0. Kill the unit if health reaches 0
	Health = FMath::Clamp(Health - FMath::TruncToInt(DamageAmount), 0, MaxHealth);
	if (Health == 0) {
		Death();
	}
	return Health;
}


// Applies damage to a given unit
// - Validation
bool AGridUnit::DamageTarget_Validate(AActor* DamagedActor, float Damage)
{
	return true;
}

// - Implementation
void AGridUnit::DamageTarget_Implementation(AActor* DamagedActor, float Damage)
{
	TSubclassOf<UDamageType> DamageType;
	UGameplayStatics::ApplyDamage(DamagedActor, Damage, nullptr, nullptr, DamageType);
}


// Select this unit so that the controlling player can use it
void AGridUnit::SelectUnit()
{
	bSelected = true;
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	PlayerController->UnitSelected(this);
	ShowNavigableLocations(CurrentTile);
	CancelTargetting();
}

// Deselect this unit so that it cannot be used
void AGridUnit::DeselectUnit()
{
	bSelected = false;

	//Clear reference in player controller
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	PlayerController->UnitSelected(nullptr);

	// Remove all tile highlighting associated with this unit
	CancelAllNavigableLocations();
	CancelTargetting();
}

// Determines whether this unit is capable of moving to a given tile
ENavigationEnum AGridUnit::CanReachTile(AMapTile* Destination)
{
	if (Destination->GetOccupyingUnit() != nullptr) { // Cannot reach tile occupied by another unit
		return ENavigationEnum::Nav_Unreachable;
	}
	
	if (Destination->GetBlockage() != nullptr) {
		if (Destination->GetBlockage()->bObstructAllMovement) { // Cannot reach tile occupied by a blockage that obstructs movement
			return ENavigationEnum::Nav_Unreachable;
		}
	}

	if (MovesRemaining < 1) { // Cannot reach tile if unit does not have enough movement points
		return ENavigationEnum::Nav_Unreachable;
	}

	if (Destination->GetActorLocation().Z > CurrentTile->GetActorLocation().Z + MoveClimbHeight) { // Cannot reach tile if tile is higher than unit can climb
		return ENavigationEnum::Nav_Unreachable;
	}
	else if (Destination->GetActorLocation().Z < CurrentTile->GetActorLocation().Z - MoveClimbHeight) { // Unit will take damage moving to a tile that is lower than the unit can climb
		return ENavigationEnum::Nav_Dangerous;
	}

	return ENavigationEnum::Nav_Safe; // Otherwise, tile is reachable and safe
}

// Highlights tiles this unit can reach when starting from a given tile
// - Validation
bool AGridUnit::ShowNavigableLocations_Validate(AMapTile* FromTile)
{
	return true;
}

// - Implementation
void AGridUnit::ShowNavigableLocations_Implementation(AMapTile* FromTile)
{
	// Remove navigation options that are now obsolete and start fresh
	CancelAllNavigableLocations();

	// Calculate number of movement points needed to leave current tile
	int MovesRequired = 0;
	if (bIgnoresBlockageSlowing) {
		MovesRequired = 1;
	} else {
		if (CurrentTile != nullptr) {
			MovesRequired = CurrentTile->GetTotalMovementCost();
		}
	}

	// Determine whether neighbouring tailes can be reached
	ATacticalControllerBase* PlayerController = Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController());
	if (MovesRequired <= MovesRemaining && PlayerController->bTurn) {
		TArray<AMapTile*> TilesInRange = FromTile->GetFourNeighbouringTiles();
		for (int i = 0; i < TilesInRange.Num(); i++) {
			AMapTile* Tile = TilesInRange[i];
			Tile->ECurrentlyNavigable = CanReachTile(Tile);
			Tile->SetHighlightMaterial();
		}
	}
}


// Cancel unit movement by setting all map tiles as unreachable
void AGridUnit::CancelAllNavigableLocations()
{
	for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->ECurrentlyNavigable = ENavigationEnum::Nav_Unreachable;
		Itr->SetHighlightMaterial();
	}
}

// Examine a tile to determine whether it can be targeted by an ability searching for suitable targets
bool AGridUnit::CheckTileForProperties(AMapTile* Tile, bool bCheckSelf, bool bCheckEnemies, bool bCheckAllies, bool bCheckEmptyTiles, bool bCheckAlliedPortal, bool bCheckEnemyPortal, bool bCheckBlockages)
{
	
	//Check for Self
	bool bSelfFound = false;
	if (bCheckSelf) {
		bSelfFound = (CurrentTile == Tile);
	}

	//Check for Enemies
	bool bEnemyFound = false;
	if (bCheckEnemies) {
		if ((CurrentTile != Tile) && (Tile->GetOccupyingUnit() != nullptr)) {
			bEnemyFound = ((Tile->GetOccupyingUnit()->Team != Team) && !(Cast<AUnit_Portal>(Tile->GetOccupyingUnit())));
		}
	}

	//Check for Allies
	bool bAllyFound = false;
	if (bCheckAllies) {
		if ((CurrentTile != Tile) && (Tile->GetOccupyingUnit() != nullptr)) {
			bAllyFound = ((Tile->GetOccupyingUnit()->Team == Team) && !(Cast<AUnit_Portal>(Tile->GetOccupyingUnit())));
		}
	}

	//Check for Empty Tile
	bool bEmptyFound = false;
	if (bCheckEmptyTiles) {
		bEmptyFound = (Tile->GetOccupyingUnit() == nullptr && Tile->GetBlockage() == nullptr);
	}

	//Check for Allied Portal
	bool bAlliedPortalFound = false;
	if (bCheckAlliedPortal) {
		if (Tile->GetOccupyingUnit() != nullptr) {
			bAlliedPortalFound = ((Tile->GetOccupyingUnit()->Team == Team) && (Cast<AUnit_Portal>(Tile->GetOccupyingUnit())));
		}
	}

	//Check for Enemy Portal
	bool bEnemyPortalFound = false;
	if (bCheckEnemyPortal) {
		if (Tile->GetOccupyingUnit() != nullptr) {
			bEnemyPortalFound = ((Tile->GetOccupyingUnit()->Team != Team) && (Cast<AUnit_Portal>(Tile->GetOccupyingUnit())));
		}
	}

	//Check for Blockage
	bool bBlockageFound = false;
	if (bCheckBlockages) {
		bBlockageFound = (Tile->GetBlockage() != nullptr);
	}

	// Check is successful if one of the sought-after properties exists on the tile
	return (bSelfFound || bEnemyFound || bAllyFound || bEmptyFound || bAlliedPortalFound || bEnemyPortalFound || bBlockageFound);
}

// Use a short line trace to determine which tile this unit is occupying
void AGridUnit::DetermineCurrentTile() {
	FHitResult Hit(ForceInit);
	FCollisionQueryParams CollisionParams;
	FVector StartPoint = this->GetActorLocation();
	FVector EndPoint = StartPoint + FVector(0.0f, 0.0f, -500.0f);
	
	GetWorld()->LineTraceSingleByObjectType(Hit, StartPoint, EndPoint, ECC_WorldDynamic, CollisionParams);

	if (Cast<AMapTile>(Hit.GetActor())) {
		// Make relationships and ensure unit is correctly positioned
		CurrentTile = Cast<AMapTile>(Hit.GetActor());
		CurrentTile->SetOccupyingUnit(this);
		SetActorLocation(CurrentTile->GetActorLocation());
	}
	
}

// Called when an ability is selected for use, but has not yet been given a target to cast on
void AGridUnit::SelectAbility(int Ability)
{
	CancelAllNavigableLocations();
	CancelTargetting();
	SelectedAbility = Ability;
	FUnitAbility SelectedAbilityStructure = AbilitySet[Ability - 1];
	TArray<AMapTile*> TilesInRange = FindAbilityRange(SelectedAbilityStructure.Range);

	for (int i = 0; i < TilesInRange.Num(); i++)
	{
		//Tiles must pass the unique ability rule and the general CheckTileForProperties function to be targetable
		bool bAbilityRulePass = false;
		switch (SelectedAbility) {
		case 1:
			bAbilityRulePass = AbilityRule1(TilesInRange[i]);
			break;
		case 2:
			bAbilityRulePass = AbilityRule2(TilesInRange[i]);
			break;
		case 3:
			bAbilityRulePass = AbilityRule3(TilesInRange[i]);
			break;
		}

		if (bAbilityRulePass && CheckTileForProperties(TilesInRange[i], SelectedAbilityStructure.bAffectsSelf, SelectedAbilityStructure.bAffectsEnemies, SelectedAbilityStructure.bAffectsAllies, SelectedAbilityStructure.bAffectsTiles, SelectedAbilityStructure.bAffectsAlliedPortal, SelectedAbilityStructure.bAffectsEnemyPortal, SelectedAbilityStructure.bAffectsBlockages)) {
			TilesInRange[i]->bTargetable = true;
			TilesInRange[i]->SetHighlightMaterial();
		}
	}
}

// Determines the tiles within a given range of the unit
TArray<AMapTile*> AGridUnit::FindAbilityRange(int Range)
{
	// Start with current tile and expand range out recursively
	TArray<AMapTile*> TilesInRange;
	TilesInRange.Add(CurrentTile);
	TilesInRange = ExpandAbilityRange(TilesInRange, Range);
	return TilesInRange;
}

// Given an array of tile already found to be within a given range, this function recursively expands the range further until the outermost reaches of the range have been found
TArray<AMapTile*> AGridUnit::ExpandAbilityRange(TArray<AMapTile*> CurrentRange, int Range)
{
	TArray<AMapTile*> TilesToAdd;
	
	// Check all tiles currently in range and add any orthogonally adjacent tiles that have not already been accounted for
	for (int i = 0; i < CurrentRange.Num(); i++) {
		TArray<AMapTile*> Neighbours = CurrentRange[i]->GetFourNeighbouringTiles();
		for (int j = 0; j < Neighbours.Num(); j++) {
			TilesToAdd.AddUnique(Neighbours[j]);
		}
		
	}
	
	// If there is more than one expansion iteration left to go, call the function again and expand the range further
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

// Cancels any targets highlighted by as targets for an ability
void AGridUnit::CancelTargetting()
{
	SelectedAbility = 0;
	for (TActorIterator<AMapTile> Itr(GetWorld()); Itr; ++Itr)
	{
		Itr->bTargetable = false;
		Itr->SetHighlightMaterial();
	}
}


// Spawn a new blockage and assign it to a tile
// - Validate
bool AGridUnit::PlaceBlockageOnTile_Validate(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass) {
	return true;
}

// - Implementation
void AGridUnit::PlaceBlockageOnTile_Implementation(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass) {
	FVector Location = Tile->GetActorLocation();
	FRotator Rotation = Tile->GetActorRotation();
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn and assign
	ABlockageC* NewBlockage = GetWorld()->SpawnActor<ABlockageC>(BlockageClass, Location, Rotation, SpawnParameters);
	Tile->SetBlockage(NewBlockage);
}


// Remove the blockage currently existing on a tile
// - Validation
bool AGridUnit::DestroyBlockageOnTile_Validate(AMapTile* Tile) {
	return true;
}

// - Implementation
void AGridUnit::DestroyBlockageOnTile_Implementation(AMapTile* Tile) {
	Tile->GetBlockage()->Destroy();
}


// Use the selected ability with a given tile as the target
// - Validate
bool AGridUnit::UseSelectedAbility_Validate(AMapTile* TargetTile)
{
	return true;
}

// - Implementation
void AGridUnit::UseSelectedAbility_Implementation(AMapTile* TargetTile)
{
	switch (SelectedAbility) {
	case 1:
		Ability1(TargetTile);
		break;
	case 2:
		Ability2(TargetTile);
		break;
	case 3:
		Ability3(TargetTile);
		break;
	}

	// Spend energy for selected ability
	Energy = FMath::Clamp(Energy - (AbilitySet[SelectedAbility - 1].Cost), 0, MaxEnergy);
	CancelTargetting();
}

// The precise functionality of the first ability in this unit's AbilitySet array (index 0)
void AGridUnit::Ability1(AMapTile* TargetTile)
{
	// To be extended in subclasses
}

// The precise functionality of the second ability in this unit's AbilitySet array (index 1)
void AGridUnit::Ability2(AMapTile* TargetTile)
{
	// To be extended in subclasses
}

// The precise functionality of the third ability in this unit's AbilitySet array (index 2)
void AGridUnit::Ability3(AMapTile* TargetTile)
{
	// To be extended in subclasses
}

// An additional rule that determines whether the given tile can be affected by the second ability in this unit's AbilitySet array (index 1)
bool AGridUnit::AbilityRule1(AMapTile* TargetTile)
{
	return true;
}

// An additional rule that determines whether the given tile can be affected by the second ability in this unit's AbilitySet array (index 1)
bool AGridUnit::AbilityRule2(AMapTile* TargetTile)
{
	return true;
}

// An additional rule that determines whether the given tile can be affected by the second ability in this unit's AbilitySet array (index 1)
bool AGridUnit::AbilityRule3(AMapTile* TargetTile)
{
	return true;
}


// This function is called when a new turn starts for this unit in order to reset it for use.
// - Validation
bool AGridUnit::NewTurnStart_Validate()
{
	return true;
}

// - Implementation
void AGridUnit::NewTurnStart_Implementation()
{
	ResetMovesAndEnergy();
}


// Runs on all clients. Ensures that this unit starts each turn with the correct number of movement and energy points.
// - Validation
bool AGridUnit::ResetMovesAndEnergy_Validate()
{
	return true;
}

// - Implementation
void AGridUnit::ResetMovesAndEnergy_Implementation()
{
	// Set movement and energy points to maximum values
	MovesRemaining = TurnMoveDistance;
	Energy = MaxEnergy;
}


// These functions funnel a call to spawn particles from the client, through the server and out to all connected clients, so that all players can see the particles that spawn
// - Validation
bool AGridUnit::CallForParticles_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

// - Implementation
void AGridUnit::CallForParticles_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	SpawnParticleEffectServer_Implementation(EmitterTemplate, Location);
}

// These functions funnel a call to spawn particles from the client, through the server and out to all connected clients, so that all players can see the particles that spawn
// - Validation
bool AGridUnit::SpawnParticleEffectServer_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

// - Implementation
void AGridUnit::SpawnParticleEffectServer_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	SpawnParticleEffectMulticast_Implementation(EmitterTemplate, Location);
}

// These functions funnel a call to spawn particles from the client, through the server and out to all connected clients, so that all players can see the particles that spawn
// - Validation
bool AGridUnit::SpawnParticleEffectMulticast_Validate(UParticleSystem* EmitterTemplate, FVector Location)
{
	return true;
}

// - Implementation
void AGridUnit::SpawnParticleEffectMulticast_Implementation(UParticleSystem* EmitterTemplate, FVector Location)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplate, Location, FRotator::ZeroRotator, true);
}


// Set the material for the TeamPlane mesh to correctly represent the appropriate team
void AGridUnit::SetTeamLightColour()
{
	FLocalPlayerContext Context = FLocalPlayerContext(GetWorld()->GetFirstPlayerController());
	ATacticalGameState* GameState = Context.GetGameState<ATacticalGameState>();
	DynMaterial = TeamPlane->CreateDynamicMaterialInstance(0, TeamPlane->GetMaterial(0));

	if (DynMaterial != nullptr) {
		DynMaterial->SetVectorParameterValue("TeamColour", GameState->GetTeamColour(Team));
	}
}


// Called when a dissolve animation timeline ticks to update the dynamic material of the unit mesh
// - Validation
bool AGridUnit::DissolveTick_Validate(float Value)
{
	return true;
}

// - Implementation
void AGridUnit::DissolveTick_Implementation(float Value)
{
	// Make a dynamic material from the mesh default material. If compatible with the animation, it will perform the dissolve. Otherwise it will remain static.
	if (Health == 0) {
		DynMaterial = UnitMesh->CreateDynamicMaterialInstance(0, DeathDissolveMaterial);
	}
	else {
		DynMaterial = UnitMesh->CreateDynamicMaterialInstance(0, UnitMesh->GetMaterial(0));
	}

	if (DynMaterial != nullptr) {
		DynMaterial->SetScalarParameterValue("Length", Value);
	}
}


// Called when the death animation finishes. Used to remove the unit from the world
void AGridUnit::DeathAnimationComplete()
{
	Destroy();
}


//Getters and Setters

// Returns the unit's current health
int AGridUnit::GetHealth()
{
	return Health;
}

// Sets whether the unit is selected or not
void AGridUnit::SetSelected(bool bNewValue) {
	bSelected = bNewValue;
}

// Returns true if the unit is currently selected. Otherwise, returns false
bool AGridUnit::GetSelected() {
	return bSelected;
}

// Sets the number of remaining movement points this unit has to an absolute value
void AGridUnit::SetMovesRemaining(int NewMoves)
{
	MovesRemaining = NewMoves;
}

// Reduces the number of movement points this unit has by a given value, so a minimum of 0
void AGridUnit::SpendMoves(int Reduction)
{
	MovesRemaining = FMath::Clamp(MovesRemaining - Reduction, 0, TurnMoveDistance);
}

// Returns the number of movement points this unit has remaining
int AGridUnit::GetMovesRemaining()
{
	return MovesRemaining;
}

// Sets the number of remaining energy points this unit has to an absolute value
void AGridUnit::SetEnergy(int NewEnergy)
{
	Energy = NewEnergy;
}

// Reduces the number of energy points this unit has by a given value, so a minimum of 0
void AGridUnit::SpendEnergy(int Reduction)
{
	Energy = FMath::Clamp(Energy - Reduction, 0 , MaxEnergy);
}

// Returns the number of energy points this unit has remaining
int AGridUnit::GetEnergy()
{
	return Energy;
}

// Returns a pointer to the tile currently occupied by this unit
AMapTile* AGridUnit::GetCurrentTile()
{
	return CurrentTile;
}

// Returns the distance this unit can climb up and down
float AGridUnit::GetMoveClimbHeight()
{
	return MoveClimbHeight;
}

// Sets which team this unit belongs to
void AGridUnit::SetTeam(int NewTeam)
{
	Team = NewTeam;
}

// Returns the integer of the team this unit belongs to
int AGridUnit::GetTeam() {
	return Team;
}

// Returns true if this unit cannot be slowed by blockages. Otherwise, returns false
bool AGridUnit::DoesIgnoreBlockageSlowing()
{
	return bIgnoresBlockageSlowing;
}
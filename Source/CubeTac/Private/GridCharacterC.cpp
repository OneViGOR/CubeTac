// Copyright 2019 James Vigor. All Rights Reserved.


#include "GridCharacterC.h"
#include "PortalC.h"
#include "TacticalControllerBase.h"
#include "TacticalGameState.h"
#include "PlayerPawnC.h"
#include "UnrealNetwork.h"
#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"


// Sets default values
AGridCharacterC::AGridCharacterC()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

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
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	//Timeline
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));
	const ConstructorHelpers::FObjectFinder<UCurveFloat> DissolveCurve(TEXT("CurveFloat'/Game/GameData/Curves/1Sec_1to0.1Sec_1to0'"));
	if (DissolveCurve.Succeeded()) {
		FCurve = DissolveCurve.Object;
	}
	FOnTimelineFloat ProgressFunction{};
	ProgressFunction.BindUFunction(this, FName("DissolveTick")); // The function DissolveTick gets called when the timeline updates
	DissolveTimeline->AddInterpFloat(FCurve, ProgressFunction, FName("Alpha"));

	//Particle Systems
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/CharacterDeath.CharacterDeath'"));
	if (ParticleSystemClass.Succeeded()) {
		DeathParticles = Cast<UParticleSystem>(ParticleSystemClass.Object);
	}

}

void AGridCharacterC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
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

	DissolveTimeline->SetLooping(false);
	DissolveTimeline->PlayFromStart();
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
	Destroy();
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
ENavigationEnum AGridCharacterC::CanReachTile(AMapTile* Destination)
{
	if (Destination->GetOccupyingCharacter() != nullptr) {
		return ENavigationEnum::Nav_Unreachable;
	}
	
	if (Destination->GetBlockage() != nullptr) {
		if (Destination->GetBlockage()->bObstructAllMovement) {
			return ENavigationEnum::Nav_Unreachable;
		}
	}

	if (MovesRemaining < 1) {
		return ENavigationEnum::Nav_Unreachable;
	}

	if (Destination->GetActorLocation().Z > CurrentTile->GetActorLocation().Z + MoveClimbHeight) {
		return ENavigationEnum::Nav_Unreachable;
	}
	else if (Destination->GetActorLocation().Z < CurrentTile->GetActorLocation().Z - MoveClimbHeight) {
		return ENavigationEnum::Nav_Dangerous;
	}

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
		if (CurrentTile != nullptr) {
			MovesRequired = CurrentTile->GetTotalMovementCost();
		}
	}
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

	//Check for Enemies
	bool bEnemyFound = false;
	if (bCheckEnemies) {
		if ((CurrentTile != Tile) && (Tile->GetOccupyingCharacter() != nullptr)) {
			bEnemyFound = ((Tile->GetOccupyingCharacter()->Team != Team) && !(Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}

	//Check for Allies
	bool bAllyFound = false;
	if (bCheckAllies) {
		if ((CurrentTile != Tile) && (Tile->GetOccupyingCharacter() != nullptr)) {
			bAllyFound = ((Tile->GetOccupyingCharacter()->Team == Team) && !(Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}

	//Check for Empty Tile
	bool bEmptyFound = false;
	if (bCheckEmptyTiles) {
		bEmptyFound = (Tile->GetOccupyingCharacter() == nullptr && Tile->GetBlockage() == nullptr);
	}

	//Check for Allied Portal
	bool bAlliedPortalFound = false;
	if (bCheckAlliedPortal) {
		if (Tile->GetOccupyingCharacter() != nullptr) {
			bAlliedPortalFound = ((Tile->GetOccupyingCharacter()->Team == Team) && (Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}

	//Check for Enemy Portal
	bool bEnemyPortalFound = false;
	if (bCheckEnemyPortal) {
		if (Tile->GetOccupyingCharacter() != nullptr) {
			bEnemyPortalFound = ((Tile->GetOccupyingCharacter()->Team != Team) && (Cast<APortalC>(Tile->GetOccupyingCharacter())));
		}
	}

	//Check for Blockage
	bool bBlockageFound = false;
	if (bCheckBlockages) {
		bBlockageFound = (Tile->GetBlockage() != nullptr);
	}

	bool bSuccess = (bSelfFound || bEnemyFound || bAllyFound || bEmptyFound || bAlliedPortalFound || bEnemyPortalFound || bBlockageFound);

	return bSuccess;
}

void AGridCharacterC::DetermineCurrentTile() {
	FHitResult Hit(ForceInit);
	FCollisionQueryParams CollisionParams;
	FVector StartPoint = this->GetActorLocation();
	FVector EndPoint = StartPoint + FVector(0.0f, 0.0f, -500.0f);
	
	GetWorld()->LineTraceSingleByObjectType(Hit, StartPoint, EndPoint, ECC_WorldDynamic, CollisionParams);

	if (Cast<AMapTile>(Hit.GetActor())) {
		CurrentTile = Cast<AMapTile>(Hit.GetActor());
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
			break;
		case 2:
			bAbilityRulePass = AbilityRule2();
			break;
		case 3:
			bAbilityRulePass = AbilityRule3();
			break;
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


bool AGridCharacterC::PlaceBlockageOnTile_Validate(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass) {
	return true;
}

void AGridCharacterC::PlaceBlockageOnTile_Implementation(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass) {
	FVector Location = Tile->GetActorLocation();
	FRotator Rotation = Tile->GetActorRotation();
	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABlockageC* NewBlockage = GetWorld()->SpawnActor<ABlockageC>(BlockageClass, Location, Rotation, SpawnParameters);
	Tile->Blockage = NewBlockage;
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
		break;
	case 2:
		Ability2(TargetTile);
		break;
	case 3:
		Ability3(TargetTile);
		break;
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
	FLocalPlayerContext Context = FLocalPlayerContext(GetWorld()->GetFirstPlayerController());
	ATacticalGameState* GameState = Context.GetGameState<ATacticalGameState>();
	UMaterialInstanceDynamic* DynMaterial = TeamPlane->CreateDynamicMaterialInstance(0, TeamPlane->GetMaterial(0));

	if (DynMaterial != nullptr) {
		DynMaterial->SetVectorParameterValue("TeamColour", GameState->GetTeamColour(Team+1));
	}
}

void AGridCharacterC::DissolveTick(float Value)
{
	UMaterialInstanceDynamic* DynMaterial = CharacterMesh->CreateDynamicMaterialInstance(0, CharacterMesh->GetMaterial(0));

	if (DynMaterial != nullptr) {
		DynMaterial->SetScalarParameterValue("Length", Value);
	}
}

// Getters and Setters
void AGridCharacterC::SetSelected(bool bNewValue) {
	bSelected = bNewValue;
}

bool AGridCharacterC::GetSelected() {
	return bSelected;
}

void AGridCharacterC::SetMovesRemaining(int NewMoves)
{
	MovesRemaining = NewMoves;
}

void AGridCharacterC::SpendMoves(int Reduction)
{
	MovesRemaining = FMath::Clamp(MovesRemaining - Reduction, 0, TurnMoveDistance);
}

int AGridCharacterC::GetMovesRemaining()
{
	return MovesRemaining;
}

void AGridCharacterC::SetEnergy(int NewEnergy)
{
	Energy = NewEnergy;
}

void AGridCharacterC::SpendEnergy(int Reduction)
{
	Energy = FMath::Clamp(Energy - Reduction, 0 , MaxEnergy);
}

int AGridCharacterC::GetEnergy()
{
	return Energy;
}

AMapTile* AGridCharacterC::GetCurrentTile()
{
	return CurrentTile;
}

void AGridCharacterC::SetTeam(int NewTeam)
{
	Team = NewTeam;
}

int AGridCharacterC::GetTeam() {
	return Team;
}

bool AGridCharacterC::DoesIgnoreBlockageSlowing()
{
	return bIgnoresBlockageSlowing;
}

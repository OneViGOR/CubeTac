// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Webber.h"
#include "Blockage_Web.h"
#include "PlayerPawnC.h"
#include "TacticalControllerBase.h"

AUnit_Webber::AUnit_Webber() {
	// Define unique traits for this unit
	Name = FText::FromString("Webber");
	IconAssetPath = "Texture2D'/Game/Textures/UI/AbilityIcons/Web.Web'";
	ERole = ERoleEnum::Role_ContSup;
	Tier = 1;
	Health = 75;
	MaxHealth = 75;
	MovesRemaining = 1;
	TurnMoveDistance = 1;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 150.0f;
	bIgnoresBlockageSlowing = true;
	MovementSpeed = 700.0f;

	//Unit Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> UnitIcon(*IconAssetPath);
	if (UnitIcon.Succeeded()) {
		Icon = UnitIcon.Object;
	}

	//Ability Icons
	static ConstructorHelpers::FObjectFinder<UTexture2D> BaseIcon(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'"));			// Icon for base ability, 'Pincer'
	static ConstructorHelpers::FObjectFinder<UTexture2D> Icon1(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Web.Web'"));				// Icon for first ability, 'Spin Web'
	static ConstructorHelpers::FObjectFinder<UTexture2D> Icon2(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/WebBreak.WebBreak'"));	// Icon for second ability, 'Break Web'
	static ConstructorHelpers::FObjectFinder<UTexture2D> Icon3(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Jump.Jump'"));			// Icon for third ability, 'Jump'

	//Ability Data
	BaseAbilityData = FUnitAbility("Pincer", BaseIcon.Object, 10.0f, 1, 2, true, false, true, false, false, false, true, false);        // Data structure for base ability, 'Pincer'
	AbilitySet.Add(FUnitAbility("Spin Web", Icon1.Object, 0.0f, 1, 5, true, true, true, true, true, false, false, false));				// Data structure for first ability, 'Spin Web'
	AbilitySet.Add(FUnitAbility("Break Web", Icon2.Object, 0.0f, 1, 5, true, false, false, false, false, false, false, true));			// Data structure for second ability, 'Break Web'
	AbilitySet.Add(FUnitAbility("Jump", Icon3.Object, 0.0f, 4, 5, true, false, false, false, true, false, false, false));				// Data structure for third ability, 'Jump'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	UnitMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Webber_Static.Webber_Static'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Stone_Dark.Stone_Dark'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}
}



// Replace empty default ability functions with new ones
void AUnit_Webber::BaseAbility(AMapTile* TargetTile)
{
	// 'Pincer' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), BaseAbilityData.Impact);  // Inflict damage
}

void AUnit_Webber::Ability1(AMapTile* TargetTile)
{
	// 'Spin Web' ability
	PlaceBlockageOnTile(TargetTile, ABlockage_Web::StaticClass());  // Place web at target location
}

void AUnit_Webber::Ability2(AMapTile* TargetTile)
{
	// 'Break Web' ability
	DestroyBlockageOnTile(TargetTile);  // Remove blockage at target location
}

void AUnit_Webber::Ability3(AMapTile* TargetTile)
{
	// 'Jump' ability
	Cast<APlayerPawnC>(GetWorld()->GetFirstPlayerController()->GetPawn())->MoveUnit(TargetTile, Cast<ATacticalControllerBase>(GetWorld()->GetFirstPlayerController()), false);  // Move webber to target location
}



bool AUnit_Webber::AbilityRule1(AMapTile* TargetTile) {
	return TargetTile->GetBlockage() == nullptr;   // In addition to basic rules, ensure there are explicitly no blockages at the target location
}

bool AUnit_Webber::AbilityRule2(AMapTile* TargetTile) {

	// In addition to basic rules, ensure that the blockage at the target location is a web
	if (TargetTile->GetBlockage() != nullptr) {
		return TargetTile->GetBlockage()->GetClass() == ABlockage_Web::StaticClass();
	}
	else {
		return false;
	}
}
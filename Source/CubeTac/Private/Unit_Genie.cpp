// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Genie.h"

AUnit_Genie::AUnit_Genie() {
	// Define unique traits for this unit
	Name = FText::FromString("Genie");
	IconAssetPath = "Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'";
	ERole = ERoleEnum::Role_TeamSup;
	Tier = 1;
	Health = 90;
	MaxHealth = 90;
	MovesRemaining = 3;
	TurnMoveDistance = 3;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 125.0f;
	bIgnoresBlockageSlowing = false;
	MovementSpeed = 350.0f;

	//Unit Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> UnitIcon(*IconAssetPath);
	if (UnitIcon.Succeeded()) {
		Icon = UnitIcon.Object;
	}

	//Ability Icons
	static ConstructorHelpers::FObjectFinder<UTexture2D> BaseIcon(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Bolt.Bolt'"));     // Icon for base ability, 'Staff'
	static ConstructorHelpers::FObjectFinder<UTexture2D> Icon1(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'"));		// Icon for first ability, 'Aid'

	// Ability Data
	BaseAbilityData = FUnitAbility("Staff", BaseIcon.Object, 10.0f, 1, 5, true, false, true, false, false, false, true, false);     // Data structure for base ability, 'Staff'
	AbilitySet.Add(FUnitAbility("Aid", Icon1.Object, 20.0f, 2, 3, true, true, false, true, false, false, true, false));          // Data structure for first ability, 'Aid'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Human.Human'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Genie.Colour_Genie'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitDeathMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Genie.Colour_Genie'"));
	if (UnitDeathMaterial.Succeeded()) {
		DeathDissolveMaterial = UnitDeathMaterial.Object;
	}


	//Set up asset references
	static ConstructorHelpers::FObjectFinder<UParticleSystem> AttackParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/Explosion.Explosion'"));
	if (AttackParticleSystemClass.Succeeded()) {
		AttackParticles = Cast<UParticleSystem>(AttackParticleSystemClass.Object);
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HealParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/Heal.Heal'"));
	if (HealParticleSystemClass.Succeeded()) {
		HealParticles = Cast<UParticleSystem>(HealParticleSystemClass.Object);
	}
}

// Replace empty default ability functions with new ones
void AUnit_Genie::BaseAbility(AMapTile* TargetTile)
{
	// 'Staff' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), BaseAbilityData.Impact);  // Inflict damage
}

void AUnit_Genie::Ability1(AMapTile* TargetTile)
{
	// 'Aid' ability
	CallForParticles(HealParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), -(AbilitySet[0].Impact));  // Heal
}
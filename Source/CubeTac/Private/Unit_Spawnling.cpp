// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Spawnling.h"
#include "Blockage_TreeStump.h"

AUnit_Spawnling::AUnit_Spawnling() {
	// Define unique traits for this unit
	Name = FText::FromString("Spawnling");
	IconAssetPath = "Texture2D'/Game/Textures/UI/AbilityIcons/Spawnling.Spawnling'";
	ERole = ERoleEnum::Role_General;
	Tier = 1;
	Health = 100;
	MaxHealth = 100;
	MovesRemaining = 3;
	TurnMoveDistance = 3;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 100.0f;
	bIgnoresBlockageSlowing = false;
	MovementSpeed = 550.0f;

	//Unit Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> UnitIcon(*IconAssetPath);
	if (UnitIcon.Succeeded()) {
		Icon = UnitIcon.Object;
	}

	//Ability Icons
	static ConstructorHelpers::FObjectFinder<UTexture2D> BaseIcon(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'"));     // Icon for base ability, 'Bite'
	static ConstructorHelpers::FObjectFinder<UTexture2D> Icon1(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Bolt.Bolt'"));		// Icon for first ability, 'Spit'
	//static ConstructorHelpers::FObjectFinder<UTexture2D> Icon2(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'"));		// Icon for second ability, 'Heal'
	//static ConstructorHelpers::FObjectFinder<UTexture2D> Icon3(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Spike.Spike'"));		// Icon for third ability, 'Obstruct'

	// Ability Data
	BaseAbilityData = FUnitAbility("Bite", BaseIcon.Object, 30.0f, 1, 2, true, false, true, false, false, false, true, false);      // Data structure for base ability, 'Bite'
	AbilitySet.Add(FUnitAbility("Spit", Icon1.Object, 25.0f, 2, 2, true, false, true, false, false, false, true, false));        // Data structure for first ability, 'Spit'
	//AbilitySet.Add(FUnitAbility("Heal", Icon2.Object, 30.0f, 0, 2, true, true, false, true, false, false, false, false));		  // Data structure for second ability, 'Heal'
	//AbilitySet.Add(FUnitAbility("Obstruct", Icon3.Object, 0.0f, 4, 5, true, false, false, false, true, false, false, false));	  // Data structure for third ability, 'Obstruct'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Spawnling_Static.Spawnling_Static'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Characters/SpawnlingDissolve.SpawnlingDissolve'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitDeathMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Characters/SpawnlingDeath.SpawnlingDeath'"));
	if (UnitDeathMaterial.Succeeded()) {
		DeathDissolveMaterial = UnitDeathMaterial.Object;
	}


	//Set up asset references
	static ConstructorHelpers::FObjectFinder<UParticleSystem> AttackParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/Explosion.Explosion'"));
	if (AttackParticleSystemClass.Succeeded()) {
		AttackParticles = Cast<UParticleSystem>(AttackParticleSystemClass.Object);
	}

	/*static ConstructorHelpers::FObjectFinder<UParticleSystem> HealParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/Heal.Heal'"));
	if (HealParticleSystemClass.Succeeded()) {
		HealParticles = Cast<UParticleSystem>(HealParticleSystemClass.Object);
	}*/
}

// Replace empty default ability functions with new ones
void AUnit_Spawnling::BaseAbility(AMapTile* TargetTile)
{
	// 'Bite' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), BaseAbilityData.Impact);  // Inflict damage
}

void AUnit_Spawnling::Ability1(AMapTile* TargetTile)
{
	// 'Spit' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), AbilitySet[0].Impact);  // Inflict damage
}

//void AUnit_Spawnling::Ability2(AMapTile* TargetTile)
//{
//	// 'Heal' ability
//	CallForParticles(HealParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play healing particle effect at target location
//	DamageTarget(TargetTile->GetOccupyingUnit(), -(AbilitySet[1].Impact));  // Inflict negative damage (effectively healing)
//}
//
//void AUnit_Spawnling::Ability3(AMapTile* TargetTile)
//{
//	// 'Obstruct' ability
//	PlaceBlockageOnTile(TargetTile, ABlockage_TreeStump::StaticClass());  // Place blockage at target location
//}
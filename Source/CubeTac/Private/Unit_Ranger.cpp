// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Ranger.h"

AUnit_Ranger::AUnit_Ranger() {
	// Define unique traits for this unit
	Name = FText::FromString("Ranger");
	IconAssetPath = "Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'";
	ERole = ERoleEnum::Role_Damage;
	Tier = 1;
	Health = 100;
	MaxHealth = 100;
	MovesRemaining = 3;
	TurnMoveDistance = 3;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 100.0f;
	bIgnoresBlockageSlowing = false;
	MovementSpeed = 450.0f;

	//Unit Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> UnitIcon(*IconAssetPath);
	if (UnitIcon.Succeeded()) {
		Icon = UnitIcon.Object;
	}

	//Ability Icons
	static ConstructorHelpers::FObjectFinder<UTexture2D> BaseIcon(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Bolt.Bolt'"));     // Icon for base ability, 'Swipe'

	// Ability Data
	BaseAbilityData = FUnitAbility("Swipe", BaseIcon.Object, 15.0f, 1, 3, true, false, true, false, false, false, true, false);     // Data structure for base ability, 'Swipe'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Human.Human'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Ranger.Colour_Ranger'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitDeathMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Ranger.Colour_Ranger'"));
	if (UnitDeathMaterial.Succeeded()) {
		DeathDissolveMaterial = UnitDeathMaterial.Object;
	}


	//Set up asset references
	static ConstructorHelpers::FObjectFinder<UParticleSystem> AttackParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/Explosion.Explosion'"));
	if (AttackParticleSystemClass.Succeeded()) {
		AttackParticles = Cast<UParticleSystem>(AttackParticleSystemClass.Object);
	}
}

// Replace empty default ability functions with new ones
void AUnit_Ranger::BaseAbility(AMapTile* TargetTile)
{
	// 'Thrust' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), BaseAbilityData.Impact);  // Inflict damage
}
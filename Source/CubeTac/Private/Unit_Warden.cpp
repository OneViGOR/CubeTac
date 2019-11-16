// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Warden.h"

AUnit_Warden::AUnit_Warden() {
	// Define unique traits for this unit
	Name = FText::FromString("Warden");
	IconAssetPath = "Texture2D'/Game/Textures/UI/AbilityIcons/Heal.Heal'";
	ERole = ERoleEnum::Role_Tank;
	Tier = 1;
	Health = 150;
	MaxHealth = 150;
	MovesRemaining = 2;
	TurnMoveDistance = 2;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 75.0f;
	bIgnoresBlockageSlowing = false;
	MovementSpeed = 300.0f;

	//Unit Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> UnitIcon(*IconAssetPath);
	if (UnitIcon.Succeeded()) {
		Icon = UnitIcon.Object;
	}

	//Ability Icons
	static ConstructorHelpers::FObjectFinder<UTexture2D> BaseIcon(TEXT("Texture2D'/Game/Textures/UI/AbilityIcons/Bolt.Bolt'"));     // Icon for base ability, 'Thrust'

	// Ability Data
	BaseAbilityData = FUnitAbility("Thrust", BaseIcon.Object, 20.0f, 1, 5, true, false, true, false, false, false, true, false);     // Data structure for base ability, 'Thrust'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Human.Human'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Warden.Colour_Warden'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitDeathMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Warden.Colour_Warden'"));
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
void AUnit_Warden::BaseAbility(AMapTile* TargetTile)
{
	// 'Thrust' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), BaseAbilityData.Impact);  // Inflict damage
}
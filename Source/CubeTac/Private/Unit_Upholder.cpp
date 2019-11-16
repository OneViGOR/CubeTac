// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Upholder.h"

AUnit_Upholder::AUnit_Upholder() {
	// Define unique traits for this unit
	Name = FText::FromString("Upholder");
	ERole = ERoleEnum::Role_Tank;
	Tier = 2;
	Health = 175;
	MaxHealth = 175;
	MovesRemaining = 2;
	TurnMoveDistance = 2;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 75.0f;
	bIgnoresBlockageSlowing = false;
	MovementSpeed = 350.0f;

	AbilitySet.Add(FUnitAbility("Cleave", NULL, 50.0f, 1, 5, true, false, true, false, false, false, false, false));  // Data structure for first ability, 'Cleave'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	UnitMesh->SetRelativeScale3D(FVector((1.0f, 1.0f, 1.0f)));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Human.Human'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Colour_Upholder.Colour_Upholder'"));
	if (UnitMaterial.Succeeded()) {
		UnitMesh->SetMaterial(0, UnitMaterial.Object);
	}


	//Set up asset references
	static ConstructorHelpers::FObjectFinder<UParticleSystem> AttackParticleSystemClass(TEXT("ParticleSystem'/Game/VFX/Explosion.Explosion'"));
	if (AttackParticleSystemClass.Succeeded()) {
		AttackParticles = Cast<UParticleSystem>(AttackParticleSystemClass.Object);
	}
}

void AUnit_Upholder::Ability1(AMapTile* TargetTile)
{
	// 'Fire' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingUnit()->GetActorLocation());  // Play attack particle effect at target location
	DamageTarget(TargetTile->GetOccupyingUnit(), AbilitySet[0].Impact);  // Inflict damage
}
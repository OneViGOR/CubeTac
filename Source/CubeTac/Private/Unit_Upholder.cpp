// Copyright 2019 James Vigor. All Rights Reserved.


#include "Unit_Upholder.h"

AUnit_Upholder::AUnit_Upholder() {
	// Define unique traits for this unit
	Name = FText::FromString("Upholder");
	Health = 175;
	MaxHealth = 175;
	MovesRemaining = 2;
	TurnMoveDistance = 2;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 75.0f;
	bIgnoresBlockageSlowing = false;

	AbilitySet.Add(FUnitAbility("Cleave", NULL, 50.0f, 1, 5, false, true, false, false, false, false, false));  // Data structure for first ability, 'Cleave'

	//Set up components
	//-Unit Mesh
	UnitMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	UnitMesh->SetRelativeScale3D(FVector((1.4f, 1.4f, 1.8f)));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UnitMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Spawnling_Static.Spawnling_Static'"));
	if (UnitMeshAsset.Succeeded()) {
		UnitMesh->SetStaticMesh(UnitMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Metal_Blue.Metal_Blue'"));
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
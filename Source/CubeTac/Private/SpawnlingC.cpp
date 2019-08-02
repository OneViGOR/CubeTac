// Copyright 2019 James Vigor. All Rights Reserved.


#include "SpawnlingC.h"
#include "Blockage_TreeStump.h"

ASpawnlingC::ASpawnlingC() {
	Name = FText::FromString("Spawnling");
	Health = 100;
	MaxHealth = 100;
	MovesRemaining = 3;
	TurnMoveDistance = 3;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 100.0f;
	bIgnoresBlockageSlowing = false;

	AbilitySet.Add(FCharacterAbility("Fire", NULL, 25.0f, 2, 2, false, true, false, false, false, false, false));
	AbilitySet.Add(FCharacterAbility("Heal", NULL, 30.0f, 0, 2, true, false, true, false, false, false, false));
	AbilitySet.Add(FCharacterAbility("Obstruct", NULL, 0.0f, 4, 5, false, false, false, true, false, false, false));

	//Set up components
	//-Character Mesh
	CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CharacterMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Spawnling_Static.Spawnling_Static'"));
	if (CharacterMeshAsset.Succeeded()) {
		CharacterMesh->SetStaticMesh(CharacterMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Characters/SpawnlingDissolve.SpawnlingDissolve'"));
	if (CharacterMaterial.Succeeded()) {
		CharacterMesh->SetMaterial(0, CharacterMaterial.Object);
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

void ASpawnlingC::Ability1(AMapTile* TargetTile)
{
	// 'Fire' ability
	CallForParticles(AttackParticles, TargetTile->GetOccupyingCharacter()->GetActorLocation());
	DamageTarget(TargetTile->GetOccupyingCharacter(), AbilitySet[0].Impact);
}

void ASpawnlingC::Ability2(AMapTile* TargetTile)
{
	// 'Heal' ability
	CallForParticles(HealParticles, TargetTile->GetOccupyingCharacter()->GetActorLocation());
	DamageTarget(TargetTile->GetOccupyingCharacter(), -(AbilitySet[1].Impact));
}

void ASpawnlingC::Ability3(AMapTile* TargetTile)
{
	// 'Obstruct' ability
	PlaceBlockageOnTile(TargetTile, ABlockage_TreeStump::StaticClass());
}

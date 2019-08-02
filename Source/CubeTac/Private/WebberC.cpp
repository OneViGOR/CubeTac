// Copyright 2019 James Vigor. All Rights Reserved.


#include "WebberC.h"

AWebberC::AWebberC() {
	Name = FText::FromString("Webber");
	Health = 75;
	MaxHealth = 75;
	MovesRemaining = 1;
	TurnMoveDistance = 1;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 125.0f;
	bIgnoresBlockageSlowing = true;

	AbilitySet.Add(FCharacterAbility("Spin Web", NULL, 0.0f, 1, 5, true, true, true, true, false, false, false));
	AbilitySet.Add(FCharacterAbility("Break Web", NULL, 0.0f, 1, 5, false, false, false, false, false, false, true));
	AbilitySet.Add(FCharacterAbility("Jump", NULL, 0.0f, 3, 5, false, false, false, true, false, false, false));

	//Set up components
	//-Character Mesh
	CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	CharacterMesh->SetRelativeScale3D(FVector(1.05f, 1.05f, 0.5f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CharacterMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Spawnling_Static.Spawnling_Static'"));
	if (CharacterMeshAsset.Succeeded()) {
		CharacterMesh->SetStaticMesh(CharacterMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Stone_Dark.Stone_Dark'"));
	if (CharacterMaterial.Succeeded()) {
		CharacterMesh->SetMaterial(0, CharacterMaterial.Object);
	}
}
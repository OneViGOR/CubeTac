// Copyright 2019 James Vigor. All Rights Reserved.


#include "UpholderC.h"

AUpholderC::AUpholderC() {
	Name = FText::FromString("Upholder");
	Health = 175;
	MaxHealth = 175;
	MovesRemaining = 2;
	TurnMoveDistance = 2;
	Energy = 5;
	MaxEnergy = 5;
	MoveClimbHeight = 75.0f;
	bIgnoresBlockageSlowing = false;

	AbilitySet.Add(FCharacterAbility("Cleave", NULL, 50.0f, 1, 5, false, true, false, false, false, false, false));

	//Set up components
	//-Character Mesh
	CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	CharacterMesh->SetRelativeScale3D(FVector((1.4f, 1.4f, 1.8f)));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CharacterMeshAsset(TEXT("StaticMesh'/Game/Geometry/SkeletalMeshes/Spawnling_Static.Spawnling_Static'"));
	if (CharacterMeshAsset.Succeeded()) {
		CharacterMesh->SetStaticMesh(CharacterMeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/PlainColours/Metal_Blue.Metal_Blue'"));
	if (CharacterMaterial.Succeeded()) {
		CharacterMesh->SetMaterial(0, CharacterMaterial.Object);
	}
}
// Copyright 2019 James Vigor. All Rights Reserved.


#include "Blockage_EnergyGeyser.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

ABlockage_EnergyGeyser::ABlockage_EnergyGeyser() {

	//Set up blockage details
	// Units can move through geysers as with normal tiles
	bObstructAllMovement = false;
	AdditionalMovementCost = 0;

	//Set up components
	//-Blockage Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BlockageMeshAsset(TEXT("StaticMesh'/Game/Geometry/StaticMeshes/Geyser.Geyser'"));
	if (BlockageMeshAsset.Succeeded()) {
		BlockageMesh->SetStaticMesh(BlockageMeshAsset.Object);

	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlockageMaterial(TEXT("MaterialInstanceConstant'/Game/Materials/Blockages/GeyserDissolve.GeyserDissolve'"));
	if (BlockageMaterial.Succeeded()) {
		BlockageMesh->SetMaterial(0, BlockageMaterial.Object);
	}

	//-Particle System
	GeyserParticleJet = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GeyserParticleJet"));
	GeyserParticleJet->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> GeyserParticleJetOver(TEXT("ParticleSystem'/Game/VFX/EnergyGeyser.EnergyGeyser'"));
	if (GeyserParticleJetOver.Succeeded()) {
		GeyserParticleJet->SetTemplate(GeyserParticleJetOver.Object);
	}
}
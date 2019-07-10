// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor_MapTile.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "GameFramework/Actor.h"
#include "TileMarker.generated.h"

UCLASS()
class CUBETAC_API ATileMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATileMarker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Components
	UPROPERTY()
	USceneComponent* SceneRoot;

	UPROPERTY()
	UStaticMeshComponent* MarkerMesh;
	class UParticleSystemComponent* ParticleMouseOver;
	class UParticleSystemComponent* ParticleSafe;
	class UParticleSystemComponent* ParticleDangerous;
	class UParticleSystemComponent* ParticleTarget;
	class UParticleSystemComponent* ParticleSelected;

	//Material References
	UMaterialInterface* MaterialMouseOver;
	UMaterialInterface* MaterialSafe;
	UMaterialInterface* MaterialDangerous;
	UMaterialInterface* MaterialTarget;
	UMaterialInterface* MaterialSelected;
	UMaterialInterface* MaterialFullTransparent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UpdateAppearance(AActor_MapTile* tile);

};

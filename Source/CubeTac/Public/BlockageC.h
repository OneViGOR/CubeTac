// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "BlockageC.generated.h"

UCLASS()
class CUBETAC_API ABlockageC : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlockageC();

protected:
	/**
	*   COMPONENTS
	*/

	// Root component provides transform data
	UPROPERTY()
		USceneComponent* SceneRoot;

	// The main appearance of this blockage as a static mesh
	UPROPERTY()
		UStaticMeshComponent* BlockageMesh;

	// Timeline for the spawn dissolve animation
	UPROPERTY()
		class UTimelineComponent* DissolveTimeline;

	// The curve that dictates the float value provided by the timeline
	class UCurveFloat* FCurve;
	

	/**
	*   FUNCTIONS
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/**
	*   VARIABLES
	*/

	// How much additional movement energy is required for a unit to leave the tile occupied by this blockage? Base movement cost is 1. Total movement required to leave this tile is 1 + AdditionalMovementCost.
	int AdditionalMovementCost;

	// If false, a unit can occupy the same tile as this blockage. If true, no unit can occupy the same tile as this blockage.
	bool bObstructAllMovement;


	/**
	*   FUNCTIONS
	*/

	// Called on a timeline to run the spawn dissolve animation.
	UFUNCTION()
		void DissolveTick(float Value);
};

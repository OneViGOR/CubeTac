// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockageC.generated.h"

UCLASS()
class CUBETAC_API ABlockageC : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlockageC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Components
	UPROPERTY()
		USceneComponent* SceneRoot;
	UPROPERTY()
		UStaticMeshComponent* BlockageMesh;

public:
	// Variables
	int AdditionalMovementCost;
	bool bObstructAllMovement;

};

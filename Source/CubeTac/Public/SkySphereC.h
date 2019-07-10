// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/Components/ExponentialHeightFogComponent.h"
#include "Runtime/Engine/Classes/Engine/ExponentialHeightFog.h"
#include "Engine/DataTable.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SkySphereC.generated.h"

UENUM(BlueprintType)
enum class EEnvironmentEnum : uint8 {
	Env_Woodlands		UMETA(DisplayName = "Woodlands"),
	Env_Hell			UMETA(DisplayName = "Hell"),
};

USTRUCT(BlueprintType)
struct FEnvironment : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
		UMaterialInstance *skyMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
		FLinearColor fogColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		UMaterialInstance *tileCapMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		UMaterialInstance *tileShaftMaterial;

	// Constructor
	FEnvironment() {

	}

};

UCLASS()
class CUBETAC_API ASkySphereC : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkySphereC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class UDataTable* EnvironmentData;

public:	
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;

	UPROPERTY()
		UStaticMeshComponent* SphereMesh;

	UPROPERTY(EditAnywhere)
		TSoftObjectPtr<AExponentialHeightFog> WorldFogRef;


	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void SetAtmosphere(EEnvironmentEnum Atmosphere);
};

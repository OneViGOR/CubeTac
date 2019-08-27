// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/Components/ExponentialHeightFogComponent.h"
#include "Runtime/Engine/Classes/Engine/ExponentialHeightFog.h"
#include "Runtime/Engine/Classes/Engine/DirectionalLight.h"
#include "Engine/DataTable.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SkySphereC.generated.h"

// An enumeration used to describe the different environmental themes - names are used in finding data table rows
UENUM(BlueprintType)
enum class EEnvironmentEnum : uint8 {
	Env_Woodlands		UMETA(DisplayName = "Woodlands"),
	Env_Hell			UMETA(DisplayName = "Hell"),
	Env_Polar			UMETA(DisplayName = "Polar"),
};

// A structure used in a data table to dictate the colours and materials used in each of the environmental themes
USTRUCT(BlueprintType)
struct FEnvironment : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
		UMaterialInstance *SkyMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
		FLinearColor FogColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		UMaterialInstance *TileCapMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		UMaterialInstance *TileShaftMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
		float LightIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
		FLinearColor LightColour;

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
	// A reference to the data table that holds the enivornmental colour palette
	class UDataTable* EnvironmentData;

public:	
	/**
	*   COMPONENTS
	*/

	// Root component provides transform data
	UPROPERTY()
		USceneComponent* SceneRoot;

	// An inverted sphere mesh the forms the sky
	UPROPERTY()
		UStaticMeshComponent* SphereMesh;


	/**
	*   VARIABLES
	*/

	// A reference to a directional light in the world which acts as the sun
	UPROPERTY(EditAnywhere)
		ADirectionalLight* Skylight;

	// A reference to the fog actor in the world
	UPROPERTY(EditAnywhere)
		TSoftObjectPtr<AExponentialHeightFog> WorldFogRef;


	/**
	*   FUNCTIONS
	*/

	// Set the appearance of the sky, fog and lighting based on user preference
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void SetAtmosphere(EEnvironmentEnum Atmosphere);
};

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

UENUM(BlueprintType)
enum class EEnvironmentEnum : uint8 {
	Env_Woodlands		UMETA(DisplayName = "Woodlands"),
	Env_Hell			UMETA(DisplayName = "Hell"),
	Env_Polar			UMETA(DisplayName = "Polar"),
};

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
	class UDataTable* EnvironmentData;

public:	
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;

	UPROPERTY()
		UStaticMeshComponent* SphereMesh;

	UPROPERTY(EditAnywhere)
		ADirectionalLight* Skylight;
	UPROPERTY(EditAnywhere)
		TSoftObjectPtr<AExponentialHeightFog> WorldFogRef;


	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void SetAtmosphere(EEnvironmentEnum Atmosphere);
};

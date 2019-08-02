// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockageC.h"
#include "SkySphereC.h"
#include "Runtime/Engine/Classes/Components/ChildActorComponent.h"
#include "MapTile.generated.h"

class AGridCharacterC;
class ATileMarker;
class AGameMap;

UENUM(BlueprintType)
enum class ENavigationEnum : uint8 {
	Nav_Safe			UMETA(DisplayName = "Safe"),
	Nav_Dangerous		UMETA(DisplayName = "Dangerous"),
	Nav_Unreachable		UMETA(DisplayName = "Unreachable")
};

UCLASS()
class CUBETAC_API AMapTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;

	UPROPERTY()
		UStaticMeshComponent* CapMesh;
	UPROPERTY()
		UStaticMeshComponent* ShaftMesh;
	UPROPERTY()
		UChildActorComponent* TileMarker;
	
	UPROPERTY()
		FVector2D Coordinates;

	UPROPERTY(BlueprintReadWrite, Replicated)
		AGridCharacterC* OccupyingCharacter;
	UPROPERTY(Replicated)
		AGameMap* GameMapReference;
	UPROPERTY()
		int MovementCost;
	
	UPROPERTY(Replicated)
		bool bVoid;

private:
	class UDataTable* EnvironmentData;

public:	
	//Set Up
	void SetCoordinates(AGameMap* Map, int X, int Y);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SetVoid(bool bVoidParam);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SetAtmosphere(EEnvironmentEnum Environment);
	
	//Utility
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void SetOccupyingCharacter(AGridCharacterC* NewOccupier);
	AMapTile* LineTraceForTile(FVector Start);
	int GetTotalMovementCost();
	TArray<AMapTile*> GetFourNeighbouringTiles();
	void SetHighlightMaterial();

	//Mouse Control
	void ClickedInGamePhase();
	void ClickedInPortalPlacementPhase();

	UFUNCTION()
		void OnBeginMouseOver(AActor* Component);
	UFUNCTION()
		void OnEndMouseOver(AActor* Component);
	UFUNCTION()
		void OnMouseClicked(AActor* Component, FKey ButtonPressed);

	//Getters and Setters
	AGridCharacterC* GetOccupyingCharacter();

	ABlockageC* GetBlockage();

	bool GetVoid();


	//Public Variables
	ENavigationEnum ECurrentlyNavigable = ENavigationEnum::Nav_Unreachable;
	bool bTargetable;
	bool bMouseOver;
	UPROPERTY(Replicated)
		ABlockageC* Blockage;
};

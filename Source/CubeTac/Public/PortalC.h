// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridCharacterC.h"
#include "PortalC.generated.h"

USTRUCT(BlueprintType)
struct FCharacterBuyData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
		TSubclassOf<AGridCharacterC> CharacterClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Data")
		int Price;

	// Constructors
	FCharacterBuyData() {
		CharacterClass = nullptr;
		Price = 1;
	}

	FCharacterBuyData(TSubclassOf<AGridCharacterC> ClassParam, int PriceParam) {
		CharacterClass = ClassParam;
		Price = PriceParam;
	}
};

UCLASS()
class CUBETAC_API APortalC : public AGridCharacterC
{
	GENERATED_BODY()
	
public:
	APortalC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Components
	UPROPERTY()
		UStaticMeshComponent* RiftMesh;
	UPROPERTY()
		UAudioComponent* AmbientHum;

	//Variables
	UPROPERTY()
		AGameMap* GameMapReference;
	UPROPERTY(BlueprintReadOnly)
		TArray<FCharacterBuyData> SpawnList;
	UPROPERTY(BlueprintReadOnly)
		FCharacterBuyData SelectedSpawn;
	UPROPERTY()
		int SpawnRange;
	UPROPERTY()
		int EnergyPerTurn;

public:
	UFUNCTION()
		void SpawnCharacter(AMapTile* SpawnTile);
	UFUNCTION(BlueprintCallable)
		void SelectSpawnCharacter(FCharacterBuyData SelectedData);
	
	void NewTurnStart_Implementation() override;
};

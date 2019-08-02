// Copyright 2019 James Vigor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "ParticleDefinitions.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "MapTile.h"
#include "GameFramework/Pawn.h"
#include "GridCharacterC.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAbility {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		UTexture2D *Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		float Impact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		int Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		int Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsSelf;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsAllies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsAlliedPortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsEnemyPortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsBlockages;

	// Constructors
	FCharacterAbility(FString NameParam, UTexture2D* IconParam, float ImpactParam, int RangeParam, int CostParam, bool bSelfParam, bool bEnemiesParam, bool bAlliesParam, bool bTilesParam, bool bAlliedPortalParam, bool bEnemyPortalParam, bool bBlockagesParam) {
		Name = FText::FromString(NameParam);
		Icon = IconParam;
		Impact = ImpactParam;
		Range = RangeParam;
		Cost = CostParam;
		bAffectsSelf = bSelfParam;
		bAffectsEnemies = bEnemiesParam;
		bAffectsAllies = bAlliesParam;
		bAffectsTiles = bTilesParam;
		bAffectsAlliedPortal = bAlliedPortalParam;
		bAffectsEnemyPortal = bEnemyPortalParam;
		bAffectsBlockages = bBlockagesParam;
	}

	FCharacterAbility() {
		Name = FText::FromString(FString("DefaultAbility"));
		Icon = NULL;
		Impact = 0.0f;
		Range = 0;
		Cost = 0;
		bAffectsSelf = false;
		bAffectsEnemies = false;
		bAffectsAllies = false;
		bAffectsTiles = false;
		bAffectsAlliedPortal = false;
		bAffectsEnemyPortal = false;
		bAffectsBlockages = false;
	}

};

UCLASS()
class CUBETAC_API AGridCharacterC : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGridCharacterC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Components
	UPROPERTY()
		USceneComponent* SceneRoot;

	UPROPERTY()
		UStaticMeshComponent* CharacterMesh;
	UPROPERTY()
		UStaticMeshComponent* TeamPlane;

	//Character Data
	UPROPERTY (BlueprintReadWrite)
		FText Name;
	UPROPERTY (BlueprintReadWrite, Replicated)
		int Health;
	UPROPERTY (BlueprintReadWrite)
		int MaxHealth;
	UPROPERTY (BlueprintReadWrite, Replicated)
		int MovesRemaining;
	UPROPERTY (BlueprintReadWrite)
		int TurnMoveDistance;
	UPROPERTY (BlueprintReadWrite, Replicated)
		int Energy;
	UPROPERTY (BlueprintReadWrite, Replicated)
		int MaxEnergy;


	UPROPERTY (BlueprintReadWrite, Replicated)
		AMapTile* CurrentTile;
	UPROPERTY (BlueprintReadWrite)
		float MoveClimbHeight;
	UPROPERTY (BlueprintReadWrite)
		bool bSelected;
	UPROPERTY (BlueprintReadWrite)
		TArray<FCharacterAbility> AbilitySet;
	UPROPERTY (BlueprintReadWrite)
		int SelectedAbility;
	UPROPERTY (BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Replicated)
		int Team = 1;
	UPROPERTY (BlueprintReadWrite)
		bool bIgnoresBlockageSlowing;

	UPROPERTY ()
	class UTimelineComponent* DissolveTimeline;
	class UCurveFloat* FCurve;
	

	TSubclassOf<class UParticleSystem> Particle;
	UParticleSystem* DeathParticles;

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Health
	void Death();
	float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) override;
	UFUNCTION (Server, Reliable, WithValidation)
		void DamageTarget(AActor* DamagedActor, float Damage);

	// Selection
	void SelectCharacter();
	UFUNCTION(BlueprintCallable)
		void DeselectCharacter();

	// Navigation
	ENavigationEnum CanReachTile(AMapTile* Destination);
	UFUNCTION (Client, Reliable, WithValidation)
		void ShowNavigableLocations(AMapTile* FromTile);
	void CancelAllNavigableLocations();
	bool CheckTileForProperties(AMapTile* Tile, bool bCheckSelf, bool bCheckEnemies, bool bCheckAllies, bool bCheckEmptyTiles, bool bCheckAlliedPortal, bool bCheckEnemyPortal, bool bCheckBlockages);
	void DetermineCurrentTile();

	// Abilities
	UFUNCTION(BlueprintCallable)
		void SelectAbility(int Ability);
	TArray<AMapTile*> FindAbilityRange(int Range);
	TArray<AMapTile*> ExpandAbilityRange(TArray<AMapTile*> CurrentRange, int Range);
	void CancelTargetting();

	UFUNCTION(Server, Reliable, WithValidation)
		void PlaceBlockageOnTile(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass);
	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyBlockageOnTile(AMapTile* Tile);

	UFUNCTION (Client, Reliable, WithValidation)
		void UseSelectedAbility(AMapTile* TargetTile);

	virtual void Ability1(AMapTile* TargetTile);
	virtual void Ability2(AMapTile* TargetTile);
	virtual void Ability3(AMapTile* TargetTile);

	virtual bool AbilityRule1();
	virtual bool AbilityRule2();
	virtual bool AbilityRule3();

	// Turn Management
	UFUNCTION (Server, Reliable, WithValidation)
		virtual void NewTurnStart();
	UFUNCTION (NetMulticast, Reliable, WithValidation)
		void ResetMovesAndEnergy();

	// Cosmetic
	UFUNCTION(Client, Unreliable, WithValidation)
		void CallForParticles(UParticleSystem* EmitterTemplate, FVector Location);
	UFUNCTION(Server, Unreliable, WithValidation)
		void SpawnParticleEffectServer(UParticleSystem* EmitterTemplate, FVector Location);
	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void SpawnParticleEffectMulticast(UParticleSystem* EmitterTemplate, FVector Location);

	void SetTeamLightColour();

	UFUNCTION()
	void DissolveTick(float Value);


	//Getters and Setters
	void SetSelected(bool bNewValue);
	bool GetSelected();

	void SetMovesRemaining(int NewMoves);
	void SpendMoves(int Reduction);
	int GetMovesRemaining();

	void SetEnergy(int NewEnergy);
	void SpendEnergy(int Reduction);
	int GetEnergy();

	AMapTile* GetCurrentTile();

	void SetTeam(int NewTeam);
	int GetTeam();

	bool DoesIgnoreBlockageSlowing();
};
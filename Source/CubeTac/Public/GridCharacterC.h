// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "ParticleDefinitions.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Actor_MapTile.h"
#include "GameFramework/Pawn.h"
#include "GridCharacterC.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAbility {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		FString Name;

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

	// Constructor
	FCharacterAbility() {
		Name = "DefaultAbility";
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
		FString Name;
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
		AActor_MapTile* CurrentTile;
	UPROPERTY (BlueprintReadWrite)
		float MoveClimbHeight;
	UPROPERTY (BlueprintReadWrite)
		bool bSelected;
	UPROPERTY (BlueprintReadWrite)
		TArray<FCharacterAbility> AbilitySet;
	UPROPERTY (BlueprintReadWrite)
		int SelectedAbility;
	UPROPERTY (BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Replicated)
		int Team;
	UPROPERTY (BlueprintReadWrite)
		bool bIgnoresBlockageSlowing;

	UPROPERTY ()
		FTimeline SpawnDissolveTimeline;

	TSubclassOf<class UParticleSystem> Particle;
	UParticleSystem* DeathParticles;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Health
	void Death();
	float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) override;
	UFUNCTION (Server, Reliable, WithValidation)
		void DamageTarget(AActor* DamagedActor, float Damage);

	// Selection
	void SelectCharacter();
	void DeselectCharacter();

	// Navigation
	ENavigationEnum CanReachTile(AActor_MapTile* Destination);
	UFUNCTION (Client, Reliable, WithValidation)
		void ShowNavigableLocations(AActor_MapTile* FromTile);
	void CancelAllNavigableLocations();
	bool CheckTileForProperties(AActor_MapTile* Tile, bool bCheckSelf, bool bCheckEnemies, bool bCheckAllies, bool bCheckEmptyTiles, bool bCheckAlliedPortal, bool bCheckEnemyPortal, bool bCheckBlockages);
	void DetermineCurrentTile();

	// Abilities
	void SelectAbility(int Ability);
	TArray<AActor_MapTile*> FindAbilityRange(int Range);
	TArray<AActor_MapTile*> ExpandAbilityRange(TArray<AActor_MapTile*> CurrentRange, int Range);
	void CancelTargetting();

	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyBlockageOnTile(AActor_MapTile* Tile);

	UFUNCTION (Client, Reliable, WithValidation)
		void UseSelectedAbility(AActor_MapTile* TargetTile);

	virtual void Ability1(AActor_MapTile* TargetTile);
	virtual void Ability2(AActor_MapTile* TargetTile);
	virtual void Ability3(AActor_MapTile* TargetTile);

	virtual bool AbilityRule1();
	virtual bool AbilityRule2();
	virtual bool AbilityRule3();

	// Turn Management
	UFUNCTION (Server, Reliable, WithValidation)
		void NewTurnStart();
	UFUNCTION (NetMulticast, Reliable, WithValidation)
		void ResetMovesAndEnergy();

	// Cosmetic
	UFUNCTION(Client, Unreliable, WithValidation)
		void CallForParticles(UParticleSystem* EmitterTemplate, FVector Location);
	UFUNCTION(Server, Unreliable, WithValidation)
		void SpawnParticleEffectServer(UParticleSystem* EmitterTemplate, FVector Location);
	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void SpawnParticleEffectMulticast(UParticleSystem* EmitterTemplate, FVector Location);

	void SpawnDissolve();
	void SetTeamLightColour();

	void DissolveTick(float Value);


	//Getters and Setters
	void SetSelected(bool bNewValue);
	bool GetSelected();

	int GetTeam();
};



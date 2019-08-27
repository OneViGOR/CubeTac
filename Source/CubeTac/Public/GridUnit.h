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
#include "GridUnit.generated.h"

// An enumeration to define a unit's role in battle for the players
UENUM(BlueprintType)
enum class ERoleEnum : uint8 {
	Role_General		UMETA(DisplayName = "General"),
	Role_Tank			UMETA(DisplayName = "Tank"),
	Role_Damage			UMETA(DisplayName = "Damage"),
	Role_TeamSup		UMETA(DisplayName = "Team Support"),
	Role_ContSup		UMETA(DisplayName = "Control Support")
};

// An enumeration to hold basic information about a unit's ability
USTRUCT(BlueprintType)
struct FUnitAbility {
	GENERATED_USTRUCT_BODY()

	// The name of the ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		FText Name;

	// A pointer to the icon displayed on this ability's button
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		UTexture2D *Icon;

	// A raw number to indicate the effectiveness of this ability - could be used to dictate the damage of an attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		float Impact;

	// The effective range of this ability in tiles measured out from the unit - minimum 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		int Range;

	// The amount of energy that must be expended to use this ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		int Cost;

	// Can this ability target the unit casting it?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsSelf;

	// Can this ability target enemy units?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsEnemies;
	
	// Can this ability target allied units?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsAllies;

	// Can this ability target empty tiles?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsTiles;

	// Can this ability target the allied portal?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsAlliedPortal;

	// Can this ability target enemy portals?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsEnemyPortal;

	// Can this ability target blockages?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
		bool bAffectsBlockages;

	/**
	*	Runs the map building algorithm with new settings
	*
	*	@Param		NameParam				The name of the ability
	*	@Param		IconParam				A pointer to the icon displayed on this ability's button
	*	@Param		ImpactParam				A raw number to indicate the effectiveness of this ability - could be used to dictate the damage of an attack
	*	@Param		RangeParam				The effective range of this ability in tiles measured out from the unit - minimum 1
	*	@Param		CostParam				The amount of energy that must be expended to use this ability
	*	@Param		bSelfParam				Can this ability target the unit casting it?
	*	@Param		bEnemiesParam			Can this ability target enemy units?
	*	@Param		bAlliesParam			Can this ability target allied units?
	*	@Param		bTilesParam				Can this ability target empty tiles?
	*	@Param		bAlliedPortalParam		Can this ability target the allied portal?
	*	@Param		bEnemyPortalParam		Can this ability target enemy portals?
	*	@Param		bBlockagesParam			Can this ability target blockages?
	*/
	FUnitAbility(FString NameParam, UTexture2D* IconParam, float ImpactParam, int RangeParam, int CostParam, bool bSelfParam, bool bEnemiesParam, bool bAlliesParam, bool bTilesParam, bool bAlliedPortalParam, bool bEnemyPortalParam, bool bBlockagesParam) {
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

	// Default constructor
	FUnitAbility() {
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
class CUBETAC_API AGridUnit : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGridUnit();

protected:
	/**
	*   FUNCTIONS
	*/

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/**
	*   COMPONENTS
	*/

	// Root component provides transform data
	UPROPERTY()
		USceneComponent* SceneRoot;

	// The main appearance of this blockage as a static mesh
	UPROPERTY()
		UStaticMeshComponent* UnitMesh;

	// The plane underneath the unit which provides a visual representation of which team it belongs to
	UPROPERTY()
		UStaticMeshComponent* TeamPlane;

	/**
	*   VARIABLES
	*/

	// The unit's display name
	UPROPERTY (BlueprintReadWrite)
		FText Name;

	// The file path to the icon that represents this unit
	UPROPERTY(BlueprintReadWrite)
		FString IconAssetPath;

	// A pointer to the icon that represents this unit
	UPROPERTY ()
		UTexture2D* Icon;

	// An enumeration that represents this unit's role in battle
	UPROPERTY()
		ERoleEnum ERole;

	// This unit's current health
	UPROPERTY (BlueprintReadWrite, Replicated)
		int Health;

	// The maximum health this unit can have
	UPROPERTY (BlueprintReadWrite)
		int MaxHealth;

	// The number of movement points this unit currently has
	UPROPERTY (BlueprintReadWrite, Replicated)
		int MovesRemaining;

	// The number of movement points this unit has at the start of each turn
	UPROPERTY (BlueprintReadWrite)
		int TurnMoveDistance;

	// The number of energy points this unit currently has for using abilities
	UPROPERTY (BlueprintReadWrite, Replicated)
		int Energy;

	// The number of energy points this unit has for using abilities at the start of each turn
	UPROPERTY (BlueprintReadWrite, Replicated)
		int MaxEnergy;


	// A pointer to the tile this unit is currently occupying
	UPROPERTY (BlueprintReadWrite, Replicated)
		AMapTile* CurrentTile;

	// The height difference between tiles that this unit can manage. A unit is unable to climb up further than this distance in a single move. Falling further than this distance in a single move results in the unit taking fall damage.
	UPROPERTY (BlueprintReadWrite)
		float MoveClimbHeight;

	// True if this unit is currently selected
	UPROPERTY (BlueprintReadWrite)
		bool bSelected;

	// An array of basic data describing this unit's list of abilities
	UPROPERTY (BlueprintReadWrite)
		TArray<FUnitAbility> AbilitySet;

	// The index for the currently selected ability
	UPROPERTY (BlueprintReadWrite)
		int SelectedAbility;

	// The unit's team as an integer 1-4
	UPROPERTY (BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Replicated)
		int Team = 1;

	// True if this unit is not affected by the slowing effects of blockages
	UPROPERTY (BlueprintReadWrite)
		bool bIgnoresBlockageSlowing;

	// Timeline for the spawn dissolve animation
	UPROPERTY ()
		class UTimelineComponent* SpawnDissolveTimeline;

	// The curve that dictates the float value provided by the spawn timeline
	class UCurveFloat* SpawnCurve;

	// Timeline for the death dissolve animation
	UPROPERTY()
		class UTimelineComponent* DeathDissolveTimeline;

	// The curve that dictates the float value provided by the death timeline
	class UCurveFloat* DeathCurve;

	// A reference to the dynamic material instance updated by the dissolve animations
	UMaterialInstanceDynamic* DynMaterial;

	// A reference to the material asset used by the spawn dissolve animation
	UMaterialInterface* SpawnDissolveMaterial;

	// A reference to the material asset used by the death dissolve animation
	UMaterialInterface* DeathDissolveMaterial;
	
	// A reference to the particle system asset used when the unit dies
	UParticleSystem* DeathParticles;

public:
	/**
	*   FUNCTIONS
	*/

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Handles the death of a unit
	virtual void Death();

	/**
	*	OVERRIDE - Called when damage is inflicted upon this unit
	*
	*	@Param		DamageAmount			The amount of damage this unit receives
	*	@Param		DamageEvent				Data about the damage event
	*	@Param		EventInstigator			The controller that instigated the damage
	*	@Param		DamageCauser			The actor that caused the damage
	*/
	float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) override;
	
	/**
	*	Applies damage to a given unit
	*
	*	@Param		DamagedActor			The actor that will receive the damage
	*	@Param		Damage					The amount of damage to be inflicted upon DamagedActor
	*/
	UFUNCTION (Server, Reliable, WithValidation)
		void DamageTarget(AActor* DamagedActor, float Damage);

	// Select this unit so that the controlling player can use it
	UFUNCTION()
		void SelectUnit();
	
	// Deselect this unit so that it cannot be used
	UFUNCTION(BlueprintCallable)
		virtual void DeselectUnit();

	/**
	*	Determines whether this unit is capable of moving to a given tile
	*
	*	@Param		Destination			The tile being considered
	*/
	UFUNCTION()
		ENavigationEnum CanReachTile(AMapTile* Destination);

	/**
	*	Highlights tiles this unit can reach when starting from a given tile
	*
	*	@Param		FromTile			The tile being considered as the starting point of movement
	*/
	UFUNCTION (Client, Reliable, WithValidation)
		void ShowNavigableLocations(AMapTile* FromTile);

	// Cancel unit movement by setting all map tiles as unreachable
	void CancelAllNavigableLocations();

	/**
	*	Examine a tile to determine whether it can be targeted by an ability searching for suitable targets
	*
	*	@Param		Tile						A pointer to the tile to check
	*	@Param		bCheckSelf					Can this ability target the unit casting it?
	*	@Param		bCheckEnemies				Can this ability target enemy units?
	*	@Param		bCheckAllies				Can this ability target allied units?
	*	@Param		bCheckEmptyTiles			Can this ability target empty tiles?
	*	@Param		bCheckAlliedPortal			Can this ability target the allied portal?
	*	@Param		bCheckEnemyPortal			Can this ability target enemy portals?
	*	@Param		bCheckBlockages				Can this ability target blockages?
	*/
	bool CheckTileForProperties(AMapTile* Tile, bool bCheckSelf, bool bCheckEnemies, bool bCheckAllies, bool bCheckEmptyTiles, bool bCheckAlliedPortal, bool bCheckEnemyPortal, bool bCheckBlockages);

	// Use a short line trace to determine which tile this unit is occupying
	void DetermineCurrentTile();

	// Called when an ability is selected for use, but has not yet been given a target to cast on
	UFUNCTION(BlueprintCallable)
		void SelectAbility(int Ability);

	// Determines the tiles within a given range of the unit
	TArray<AMapTile*> FindAbilityRange(int Range);

	// Given an array of tile already found to be within a given range, this function recursively expands the range further until the outermost reaches of the range have been found
	TArray<AMapTile*> ExpandAbilityRange(TArray<AMapTile*> CurrentRange, int Range);

	// Cancels any targets highlighted by as targets for an ability
	void CancelTargetting();

	/**
	*	Spawn a new blockage and assign it to a tile
	*
	*	@Param		Tile				The tile which the blockage will spawn upon
	*	@Param		BlockageClass		The type of blockage to spawn
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void PlaceBlockageOnTile(AMapTile* Tile, TSubclassOf<ABlockageC> BlockageClass);

	/**
	*	Remove the blockage currently existing on a tile
	*
	*	@Param		Tile			The tile from which the blockage will be removed
	*/
	UFUNCTION(Server, Reliable, WithValidation)
		void DestroyBlockageOnTile(AMapTile* Tile);

	/**
	*	Use the selected ability with a given tile as the target
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	UFUNCTION (Client, Reliable, WithValidation)
		void UseSelectedAbility(AMapTile* TargetTile);

	/**
	*	The precise functionality of the first ability in this unit's AbilitySet array (index 0)
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	virtual void Ability1(AMapTile* TargetTile);

	/**
	*	The precise functionality of the second ability in this unit's AbilitySet array (index 1)
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	virtual void Ability2(AMapTile* TargetTile);

	/**
	*	The precise functionality of the third ability in this unit's AbilitySet array (index 2)
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	virtual void Ability3(AMapTile* TargetTile);

	/**
	*	An additional rule that determines whether the given tile can be affected by the first ability in this unit's AbilitySet array (index 0)
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	virtual bool AbilityRule1(AMapTile* TargetTile);
	
	/**
	*	An additional rule that determines whether the given tile can be affected by the second ability in this unit's AbilitySet array (index 1)
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	virtual bool AbilityRule2(AMapTile* TargetTile);
	
	/**
	*	An additional rule that determines whether the given tile can be affected by the third ability in this unit's AbilitySet array (index 2)
	*
	*	@Param		TargetTile			The tile that the ability will be directed towards
	*/
	virtual bool AbilityRule3(AMapTile* TargetTile);

	// This function is called when a new turn starts for this unit in order to reset it for use.
	UFUNCTION (Server, Reliable, WithValidation)
		virtual void NewTurnStart();

	// Runs on all clients. Ensures that this unit starts each turn with the correct number of movement and energy points.
	UFUNCTION (NetMulticast, Reliable, WithValidation)
		void ResetMovesAndEnergy();

	/**
	*	These functions funnel a call to spawn particles from the client, through the server and out to all connected clients, so that all players can see the particles that spawn
	*
	*	@Param		EmitterTemplate			A pointer to the particle system requested
	*	@Param		Location				The world location the particles should spawn at
	*/
	UFUNCTION(Client, Unreliable, WithValidation)
		void CallForParticles(UParticleSystem* EmitterTemplate, FVector Location);
	UFUNCTION(Server, Unreliable, WithValidation)
		void SpawnParticleEffectServer(UParticleSystem* EmitterTemplate, FVector Location);
	UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void SpawnParticleEffectMulticast(UParticleSystem* EmitterTemplate, FVector Location);

	// Set the material for the TeamPlane mesh to correctly represent the appropriate team
	void SetTeamLightColour();

	/**
	*	Called when a dissolve animation timeline ticks to update the dynamic material of the unit mesh
	*
	*	@Param		Value			The new value for the material parameter
	*/
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void DissolveTick(float Value);
	
	// Called when the death animation finishes. Used to remove the unit from the world
	UFUNCTION()
		void DeathAnimationComplete();


	//Getters and Setters
	
	// Returns the unit's current health
	int GetHealth();

	// Sets whether the unit is selected or not
	void SetSelected(bool bNewValue);

	// Returns true if the unit is currently selected. Otherwise, returns false
	bool GetSelected();

	// Sets the number of remaining movement points this unit has to an absolute value
	void SetMovesRemaining(int NewMoves);

	// Reduces the number of movement points this unit has by a given value, so a minimum of 0
	void SpendMoves(int Reduction);

	// Returns the number of movement points this unit has remaining
	int GetMovesRemaining();

	// Sets the number of remaining energy points this unit has to an absolute value
	void SetEnergy(int NewEnergy);

	// Reduces the number of energy points this unit has by a given value, so a minimum of 0
	void SpendEnergy(int Reduction);

	// Returns the number of energy points this unit has remaining
	int GetEnergy();

	// Returns a pointer to the tile currently occupied by this unit
	AMapTile* GetCurrentTile();

	// Returns the distance this unit can climb up and down
	float GetMoveClimbHeight();

	// Sets which team this unit belongs to
	void SetTeam(int NewTeam);

	// Returns the integer of the team this unit belongs to
	int GetTeam();

	// Returns true if this unit cannot be slowed by blockages. Otherwise, returns false
	bool DoesIgnoreBlockageSlowing();
};
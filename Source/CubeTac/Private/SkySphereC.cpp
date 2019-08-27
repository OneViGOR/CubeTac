// Copyright 2019 James Vigor. All Rights Reserved.


#include "SkySphereC.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"

// Sets default values
ASkySphereC::ASkySphereC()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	//Set up components
	//-Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	//-Sphere Mesh
	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	SphereMesh->SetupAttachment(RootComponent);
	SphereMesh->SetRelativeScale3D(FVector(1000.0f, 1000.0f, 1000.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/EditorMeshes/AssetViewer/Sphere_inversenormals.Sphere_inversenormals"));
	if (SphereMeshAsset.Succeeded()) {
		SphereMesh->SetStaticMesh(SphereMeshAsset.Object);
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkyMaterial(TEXT("/Game/Materials/Sky/Sky_Woodlands.Sky_Woodlands"));
		SphereMesh->SetMaterial(0, SkyMaterial.Object);
	}
	
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Environment Data Table
	static ConstructorHelpers::FObjectFinder<UDataTable> EnvironmentDataAsset(TEXT("DataTable'/Game/Environment/Environment/EnvironmentColours.EnvironmentColours'"));

	if (EnvironmentDataAsset.Succeeded()) {
		EnvironmentData = EnvironmentDataAsset.Object;
	}
}

// Called when the game starts or when spawned
void ASkySphereC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Set the appearance of the sky, fog and lighting based on user preference
// - Validation
bool ASkySphereC::SetAtmosphere_Validate(EEnvironmentEnum Atmosphere)
{
	return true;
}

// - Implementation
void ASkySphereC::SetAtmosphere_Implementation(EEnvironmentEnum Atmosphere)
{
	// Find the name of the data rable row based on the name of the enumeration
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnvironmentEnum"), true);
	FString EnumString = EnumPtr->GetNameStringByIndex((int32)Atmosphere);
	FName EnumName = FName(*EnumString.RightChop(4));
	static const FString ContextString(TEXT("GENERAL"));

	// Isolate the data table row and store its contents
	FEnvironment* FoundData = EnvironmentData->FindRow<FEnvironment>(EnumName, ContextString, true);

	// Set materials and colours for the sky mesh, fog and lighting based on values from the data table
	SphereMesh->SetMaterial(0, (UMaterialInterface*)FoundData->SkyMaterial);
	if (WorldFogRef.IsValid()) {
		WorldFogRef->GetComponent()->SetFogInscatteringColor(FoundData->FogColour);
	}

	UDirectionalLightComponent* LightComponent = Cast<UDirectionalLightComponent>(Skylight->GetComponentByClass(UDirectionalLightComponent::StaticClass()));
	LightComponent->SetLightBrightness(FoundData->LightIntensity);
	LightComponent->UpdateColorAndBrightness();
	
}
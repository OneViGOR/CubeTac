// Fill out your copyright notice in the Description page of Project Settings.


#include "SkySphereC.h"

// Sets default values
ASkySphereC::ASkySphereC()
{
	PrimaryActorTick.bCanEverTick = false;

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
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkyMaterial(TEXT("/Game/Materials/Sky/Sky_Blue.Sky_Blue"));
		SphereMesh->SetMaterial(0, SkyMaterial.Object);
	}

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


bool ASkySphereC::SetAtmosphere_Validate(EEnvironmentEnum Atmosphere)
{
	return true;
}

void ASkySphereC::SetAtmosphere_Implementation(EEnvironmentEnum Atmosphere)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnvironmentEnum"), true);
	FString enumString = EnumPtr->GetNameStringByIndex((int32)Atmosphere);
	FName enumName = FName(*enumString.RightChop(4));
	static const FString ContextString(TEXT("GENERAL"));

	FEnvironment* foundData = EnvironmentData->FindRow<FEnvironment>(enumName, ContextString, true);

	SphereMesh->SetMaterial(0, (UMaterialInterface*)foundData->skyMaterial);
	if (WorldFogRef.IsValid()) {
		WorldFogRef->GetComponent()->SetFogInscatteringColor(foundData->fogColour);
	}
}
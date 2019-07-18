// Fill out your copyright notice in the Description page of Project Settings.


#include "TacticalControllerC.h"
#include "Blueprint/UserWidget.h"

ATacticalControllerC::ATacticalControllerC()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	bReplicates = true;
}

void ATacticalControllerC::BeginPlay()
{
	Super::BeginPlay();

	if (wMainMenu) // Check if the Asset is assigned in the blueprint.
	{
		// Create the widget and store it.
		MyMainMenu = CreateWidget<UUserWidget>(this, wMainMenu);

		// now you can use the widget directly since you have a referance for it.
		// Extra check to  make sure the pointer holds the widget.
		if (MyMainMenu)
		{
			//let add it to the view port
			MyMainMenu->AddToViewport();
		}
	}

}

bool ATacticalControllerC::SetUpGameUI_Validate() {
	return true;
}

void ATacticalControllerC::SetUpGameUI_Implementation() {

}

void ATacticalControllerC::CharacterSelected(AGridCharacterC* character)
{

}
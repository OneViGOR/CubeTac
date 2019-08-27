// Copyright 2019 James Vigor. All Rights Reserved.


#include "GameInterfaceBase.h"


void UGameInterfaceBase::UnitSelected_Implementation(AGridUnit* Unit)
{
	// Extend this in blueprints to update the interface when a character is selected/deselected
}

void UGameInterfaceBase::GenerateSpawnButtons_Implementation(AUnit_Portal* Portal)
{
	// Extend this in blueprints to spawn portal spawn buttons
}

void UGameInterfaceBase::GenerateAbilityButtons_Implementation(AGridUnit* Unit)
{
	// Extend this in blueprints to spawn character ability buttons
}

void UGameInterfaceBase::DisplayEndGameScreen_Implementation(EVictoryState VictoryState)
{
	// Extend this in blueprints to display the end of game interface
}
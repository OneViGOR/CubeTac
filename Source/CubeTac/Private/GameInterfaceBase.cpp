// Copyright 2019 James Vigor. All Rights Reserved.


#include "GameInterfaceBase.h"


void UGameInterfaceBase::CharacterSelected_Implementation(AGridCharacterC* Character)
{
	//Extend this in blueprints to update the interface when a character is selected/deselected
}

void UGameInterfaceBase::GenerateSpawnButtons_Implementation(APortalC* Portal)
{
	//Extend this in blueprints to spawn portal spawn buttons
}

void UGameInterfaceBase::GenerateAbilityButtons_Implementation(AGridCharacterC* Character)
{
	//Extend this in blueprints to spawn character ability buttons
}

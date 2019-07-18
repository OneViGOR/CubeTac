// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeTacGameModeBase.h"
#include "TacticalGameState.h"
#include "PlayerPawnC.h"

ACubeTacGameModeBase::ACubeTacGameModeBase() {

	GameStateClass = ATacticalGameState::StaticClass();
	PlayerControllerClass = ATacticalControllerC::StaticClass();
	DefaultPawnClass = APlayerPawnC::StaticClass();
}
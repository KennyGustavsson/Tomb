// Copyright Epic Games, Inc. All Rights Reserved.

#include "GP4GameMode.h"
#include "GP4HUD.h"
#include "GP4Character.h"
#include "UObject/ConstructorHelpers.h"

AGP4GameMode::AGP4GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGP4HUD::StaticClass();
}

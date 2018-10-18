// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FIT2097A2GameMode.h"
#include "FIT2097A2HUD.h"
#include "FIT2097A2Character.h"
#include "UObject/ConstructorHelpers.h"

AFIT2097A2GameMode::AFIT2097A2GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFIT2097A2HUD::StaticClass();

	//***************************************************************************************************
//** MYCODE
//***************************************************************************************************


}

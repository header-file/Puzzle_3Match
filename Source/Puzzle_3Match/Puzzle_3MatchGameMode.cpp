// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Puzzle_3MatchGameMode.h"
#include "Puzzle_3MatchPlayerController.h"
#include "Puzzle_3MatchPawn.h"

APuzzle_3MatchGameMode::APuzzle_3MatchGameMode()
{
	// no pawn by default
	DefaultPawnClass = APuzzle_3MatchPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = APuzzle_3MatchPlayerController::StaticClass();
}

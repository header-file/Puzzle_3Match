// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class Puzzle_3MatchTarget : TargetRules
{
    public Puzzle_3MatchTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("Puzzle_3Match");
	}
}

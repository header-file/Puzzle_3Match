// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class Puzzle_3MatchEditorTarget : TargetRules
{
    public Puzzle_3MatchEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("Puzzle_3Match");
	}
}

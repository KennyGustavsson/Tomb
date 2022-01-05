// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GP4 : ModuleRules
{
	public GP4(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "CableComponent" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CableComponent" });
	}
}

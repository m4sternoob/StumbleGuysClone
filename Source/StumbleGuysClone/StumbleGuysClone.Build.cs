// Copyright (c) 2026. Built for portfolio. Free to study.

using UnrealBuildTool;

public class StumbleGuysClone : ModuleRules
{
	public StumbleGuysClone(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}

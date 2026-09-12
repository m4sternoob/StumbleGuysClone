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
			"PhysicsCore",
			"NetCore",
			"GameplayTags",
			"GameplayTasks",
			"NavigationSystem",
			"AIModule"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UMG"
		});

		// Enable C++17/20 features
		CppStandard = CppStandardVersion.Cpp20;
	}
}
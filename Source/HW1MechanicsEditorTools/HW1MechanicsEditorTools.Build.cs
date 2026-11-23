using UnrealBuildTool;

public class HW1MechanicsEditorTools : ModuleRules
{
    public HW1MechanicsEditorTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "UnrealEd", "BlueprintGraph",
            "BlueprintEditorLibrary", "Kismet", "AnimGraph", "HW1Mechanics",
            "AIModule", "AIGraph", "BehaviorTreeEditor", "AssetRegistry",
            "PCG", "NavigationSystem"
        });
    }
}

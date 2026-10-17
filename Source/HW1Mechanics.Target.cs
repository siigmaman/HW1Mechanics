using UnrealBuildTool;
using System.Collections.Generic;

public class HW1MechanicsTarget : TargetRules
{
    public HW1MechanicsTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("HW1Mechanics");
    }
}

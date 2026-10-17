using UnrealBuildTool;
using System.Collections.Generic;

public class HW1MechanicsEditorTarget : TargetRules
{
    public HW1MechanicsEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("HW1Mechanics");
        ExtraModuleNames.Add("HW1MechanicsEditorTools");
    }
}

"""Create the Sprint state and transitions in ABP_Unarmed."""

import unreal

if not unreal.HWEditorSetupLibrary.add_sprint_animation():
    raise RuntimeError("Could not create the Sprint animation state")

path = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
bp = unreal.EditorAssetLibrary.load_asset(path)
if not bp or not unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False):
    raise RuntimeError("Could not save " + path)

unreal.log("HW1_SPRINT_ANIMATION_OK")

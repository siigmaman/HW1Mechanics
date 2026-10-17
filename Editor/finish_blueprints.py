"""Add the Blueprint response to the C++ interaction delegate."""

import unreal

if not unreal.HWEditorSetupLibrary.add_interaction_print():
    raise RuntimeError("Could not add the interaction Print String graph")

path = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"
bp = unreal.EditorAssetLibrary.load_asset(path)
if not bp or not unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False):
    raise RuntimeError("Could not save " + path)

unreal.log("HW1_INTERACTION_BLUEPRINT_OK")

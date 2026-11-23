"""Rebuild AI assets and navigation without replacing existing level actors."""
import unreal

if not unreal.HWEditorSetupLibrary.setup_hw2_assets():
    raise RuntimeError("Invalid HW2 runtime tree")
if not unreal.HWEditorSetupLibrary.build_hw2_navigation():
    raise RuntimeError("AI spawn is not covered by navigation")
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if isinstance(actor, unreal.RecastNavMesh):
        actor.set_editor_property("runtime_generation", unreal.RuntimeGenerationType.DYNAMIC)
        actor.set_editor_property("force_rebuild_on_load", True)
        actor.modify()
        unreal.log("HW2_SAVE_NAV: " + actor.get_path_name())
for path in ("/Game/HW2/AI/BB_Enemy", "/Game/HW2/AI/BT_Enemy", "/Game/HW2/PCG/PCG_ArenaProps"):
    if not unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + path)
if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_all_dirty_levels():
    raise RuntimeError("Could not save navigation")
unreal.log("HW2_REPAIR_OK")

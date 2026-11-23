"""Create HW2 AI/PCG assets and extend the Third Person level."""

import unreal


if not unreal.HWEditorSetupLibrary.setup_hw2_assets():
    raise RuntimeError("Could not create HW2 Blackboard, Behavior Tree and PCG graph")

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for actor in actor_subsystem.get_all_level_actors():
    if actor.get_actor_label() in {"HW2_Enemy", "HW2_NavMesh", "HW2_PCG"}:
        actor_subsystem.destroy_actor(actor)

enemy_class = unreal.load_class(None, "/Script/HW1Mechanics.HWAICharacter")
if not enemy_class:
    raise RuntimeError("HWAICharacter class is unavailable")

enemy = actor_subsystem.spawn_actor_from_class(
    enemy_class, unreal.Vector(0.0, 1200.0, 110.0), unreal.Rotator()
)
enemy.set_actor_label("HW2_Enemy")

nav_volume = actor_subsystem.spawn_actor_from_class(
    unreal.NavMeshBoundsVolume, unreal.Vector(0.0, 0.0, 200.0), unreal.Rotator()
)
nav_volume.set_actor_label("HW2_NavMesh")
nav_volume.set_actor_scale3d(unreal.Vector(25.0, 25.0, 5.0))

pcg_volume = actor_subsystem.spawn_actor_from_class(
    unreal.PCGVolume, unreal.Vector(0.0, 0.0, 60.0), unreal.Rotator()
)
pcg_volume.set_actor_label("HW2_PCG")
pcg_volume.set_actor_scale3d(unreal.Vector(24.0, 24.0, 2.0))

pcg_graph = unreal.EditorAssetLibrary.load_asset("/Game/HW2/PCG/PCG_ArenaProps")
pcg_component = pcg_volume.get_component_by_class(unreal.PCGComponent)
if not pcg_graph or not pcg_component:
    raise RuntimeError("PCG graph or PCG component is unavailable")
pcg_component.set_graph(pcg_graph)
pcg_component.generate_local(True)

for actor in (enemy, nav_volume, pcg_volume):
    try:
        actor.set_editor_property("is_spatially_loaded", False)
    except Exception:
        pass

level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.HWEditorSetupLibrary.build_hw2_navigation():
    raise RuntimeError("AI spawn is not covered by navigation")
if not level_subsystem.save_all_dirty_levels():
    raise RuntimeError("Could not save the Third Person level")

for path in (
    "/Game/HW2/AI/BB_Enemy",
    "/Game/HW2/AI/BT_Enemy",
    "/Game/HW2/PCG/PCG_ArenaProps",
):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset or not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + path)

configured_actors = {
    actor.get_actor_label(): actor
    for actor in actor_subsystem.get_all_level_actors()
    if actor.get_actor_label() in {"HW2_Enemy", "HW2_NavMesh", "HW2_PCG"}
}
if set(configured_actors) != {"HW2_Enemy", "HW2_NavMesh", "HW2_PCG"}:
    raise RuntimeError("HW2 actors were not saved correctly")

unreal.log(
    "HW2_SETUP_OK: AI=%s NavMesh=%s PCG=%s"
    % (
        configured_actors["HW2_Enemy"].get_actor_location(),
        configured_actors["HW2_NavMesh"].get_actor_location(),
        configured_actors["HW2_PCG"].get_actor_location(),
    )
)

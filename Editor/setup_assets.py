"""Finish the Third Person input and character assets in Unreal Editor.

Run with UnrealEditor-Cmd -ExecutePythonScript=... after the C++ module builds.
The script is repeatable and fails loudly if the expected template is absent.
"""

import os
import unreal


def find_by_name(name):
    matches = [
        path.split(".")[0]
        for path in unreal.EditorAssetLibrary.list_assets(
            "/Game", recursive=True, include_folder=False
        )
        if path.split(".")[0].split("/")[-1] == name
    ]
    if len(matches) != 1:
        raise RuntimeError("Expected one %s asset, found: %s" % (name, matches))
    return matches[0]


def load_or_create(name, folder, asset_class, factory):
    path = folder + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)
    unreal.EditorAssetLibrary.make_directory(folder)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, folder, asset_class, factory
    )
    if not asset:
        raise RuntimeError("Could not create " + path)
    return asset


def key(name):
    result = unreal.Key()
    result.set_editor_property("key_name", unreal.Name(name))
    return result


def add_mapping(context, action, key_name):
    for mapping in context.get_editor_property("mappings"):
        mapped_action = mapping.get_editor_property("action")
        mapped_key = mapping.get_editor_property("key")
        if mapped_action == action and str(mapped_key.get_editor_property("key_name")) == key_name:
            return
    context.modify()
    context.map_key(action, key(key_name))


imc_path = find_by_name("IMC_Default")
imc = unreal.EditorAssetLibrary.load_asset(imc_path)
action_folder = imc_path.rsplit("/", 1)[0] + "/Actions"

sprint = load_or_create(
    "IA_Sprint", action_folder, unreal.InputAction, unreal.InputAction_Factory()
)
interact = load_or_create(
    "IA_Interact", action_folder, unreal.InputAction, unreal.InputAction_Factory()
)
add_mapping(imc, sprint, "LeftShift")
add_mapping(imc, interact, "E")

assets_to_save = [sprint, interact, imc]
if os.environ.get("HW1_INPUT_ONLY") != "1":
    bp_path = find_by_name("BP_ThirdPersonCharacter")
    bp = unreal.EditorAssetLibrary.load_asset(bp_path)
    character_class = unreal.load_class(None, "/Script/HW1Mechanics.HWCharacter")
    if not character_class:
        raise RuntimeError("HWCharacter class unavailable; build the C++ module first")
    unreal.BlueprintEditorLibrary.reparent_blueprint(bp, character_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    defaults = unreal.get_default_object(bp.generated_class())
    defaults.set_editor_property("sprint_action", sprint)
    defaults.set_editor_property("interact_action", interact)
    assets_to_save.append(bp)

for asset in assets_to_save:
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError("Could not save " + asset.get_path_name())

unreal.log("HW1_SETUP_ASSETS_OK: " + ", ".join(a.get_name() for a in assets_to_save))

"""Inspect the template Animation Blueprint before adding a Sprint state."""

import unreal

path = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
bp = unreal.EditorAssetLibrary.load_asset(path)
if not bp:
    raise RuntimeError("Missing animation blueprint: " + path)

for graph in unreal.BlueprintEditorLibrary.list_graphs(bp):
    unreal.log("HW1_ANIM_GRAPH: %s (%s)" % (graph.get_name(), graph.get_class().get_name()))
    try:
        for node in graph.get_editor_property("nodes"):
            unreal.log("HW1_ANIM_NODE: %s (%s)" % (node.get_name(), node.get_class().get_name()))
    except Exception as exc:
        unreal.log_warning("HW1_ANIM_NODES_UNAVAILABLE: %s" % exc)

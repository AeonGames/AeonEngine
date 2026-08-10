# Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Batch convert Synty SIDEKICK part FBX files into AeonEngine meshes.

aeontool cannot read FBX, so geometry goes through Blender while
``aeontool sidekick`` emits the model, material and palette textures.

Every Sidekick part is skinned to the same skeleton, so the skeleton is
exported once from the first file that carries an armature.

Run headless from the repository root:

    blender -b --factory-startup -noaudio --python tools/blender/sidekick_batch.py -- \\
        --in <dir with SK_*.fbx> --out game/sidekick [--recipe <character.sk>]
"""

import math
import os
import sys

import bpy

_ADDON_MODULES = ("io_mesh_msh", "io_skeleton_skl")


def _install_addon_path():
    root = os.path.dirname(os.path.abspath(__file__))
    for folder in ("modules", "addons"):
        candidate = os.path.join(root, folder)
        if candidate not in sys.path:
            sys.path.insert(0, candidate)


def _parts_from_recipe(recipe_path):
    """Read the part names a .sk recipe lists, so both halves of the pipeline
    are driven by the same file."""
    parts = []
    in_parts = False
    with open(recipe_path, "r", encoding="utf-8") as recipe:
        for line in recipe:
            line = line.rstrip("\r\n")
            stripped = line.strip()
            if not stripped:
                continue
            if not line.startswith((" ", "\t", "-")):
                in_parts = stripped == "Parts:"
                continue
            if in_parts and stripped.startswith("- Name:"):
                parts.append(stripped.split(":", 1)[1].strip())
    return parts


def _parse_args(argv):
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    options = {"in": [], "out": None, "parts": [], "text": False, "rest_from": None}
    index = 0
    while index < len(argv):
        argument = argv[index]
        if argument in ("--in", "-i"):
            index += 1
            options["in"].append(argv[index])
        elif argument == "--rest-from":
            index += 1
            options["rest_from"] = argv[index]
        elif argument in ("--out", "-o"):
            index += 1
            options["out"] = argv[index]
        elif argument == "--recipe":
            index += 1
            options["parts"].extend(_parts_from_recipe(argv[index]))
        elif argument == "--parts":
            index += 1
            while index < len(argv) and not argv[index].startswith("-"):
                options["parts"].append(argv[index])
                index += 1
            continue
        elif argument == "--text":
            options["text"] = True
        else:
            raise SystemExit("Unknown option {}".format(argument))
        index += 1
    if not options["in"] or not options["out"]:
        raise SystemExit(
            "Usage: --in <fbx dir> [--in <fbx dir> ...] --out <asset dir> "
            "[--recipe <file.sk>] [--parts NAME ...] [--rest-from <clip.fbx>] [--text]")
    return options


def _rebind_to_rest(rig, mesh, rest_fbx):
    """Re-bind the part to the rest pose of the animation rig.

    Synty's body clips are authored on a rig that rests arms-down while the
    modular parts bind in a T-pose, and that difference lives in the clip rig's
    rest pose rather than in the animation. Unity reconciles the two through a
    humanoid Avatar; here the part is simply posed onto the clip's rest and
    re-bound there, after which a converted clip applies directly.
    """
    before = {o.name for o in bpy.data.objects}
    bpy.ops.import_scene.fbx(filepath=rest_fbx)
    imported = [o for o in bpy.data.objects if o.name not in before]
    clip = next((o for o in imported if o.type == "ARMATURE"), None)
    if clip is None:
        raise SystemExit("No armature in {}".format(rest_fbx))
    if clip.animation_data is not None:
        clip.animation_data.action = None
    normalize_rig(imported)

    clip_bones = {b.name for b in clip.data.bones}
    _select_only(rig)
    bpy.ops.object.mode_set(mode="POSE")
    for pose_bone in rig.pose.bones:
        if pose_bone.name not in clip_bones:
            continue
        # Rotation drives the pose; the part's own bone lengths then place every
        # child, which is what keeps the limbs from stretching.
        rotation = pose_bone.constraints.new("COPY_ROTATION")
        rotation.target = clip
        rotation.subtarget = pose_bone.name
        if pose_bone.parent is None:
            location = pose_bone.constraints.new("COPY_LOCATION")
            location.target = clip
            location.subtarget = pose_bone.name
    bpy.context.view_layer.update()
    bpy.ops.pose.select_all(action="SELECT")
    bpy.ops.pose.visual_transform_apply()
    for pose_bone in rig.pose.bones:
        for constraint in list(pose_bone.constraints):
            pose_bone.constraints.remove(constraint)
    bpy.ops.object.mode_set(mode="OBJECT")

    # Bake the posed deformation into the mesh before the pose becomes the rest
    # pose, keeping a copy of the modifier so the part stays skinned.
    modifier = next((m for m in mesh.modifiers if m.type == "ARMATURE"), None)
    if modifier is not None:
        _select_only(mesh)
        # Blender refuses to apply a modifier over shape keys. The Sidekick parts
        # carry face/body blend shapes, which the mesh format has no room for and
        # the exporter already ignores, so dropping them costs nothing.
        if mesh.data.shape_keys is not None:
            bpy.ops.object.shape_key_remove(all=True)
        bpy.ops.object.modifier_copy(modifier=modifier.name)
        bpy.ops.object.modifier_apply(modifier=modifier.name)

    _select_only(rig)
    bpy.ops.object.mode_set(mode="POSE")
    bpy.ops.pose.armature_apply()
    bpy.ops.object.mode_set(mode="OBJECT")

    for obj in imported:
        bpy.data.objects.remove(obj, do_unlink=True)


def _largest_mesh():
    """Some Synty parts carry a stray loose-edge mesh next to the real one."""
    meshes = [o for o in bpy.data.objects if o.type == "MESH" and len(o.data.polygons)]
    if not meshes:
        return None
    return max(meshes, key=lambda o: len(o.data.vertices))


def normalize_rig(objects):
    """Put freshly imported Sidekick objects into the engine's space.

    Scale -- Synty rigs import with a 0.01 object scale. The mesh exporter bakes
    the world matrix but the skeleton and animation exporters read armature
    local space, so without this the skeleton comes out in centimetres against
    metre meshes: invisible in bind pose, a 100x explosion once a clip drives it.

    Facing -- the engine's forward vector is +Y and Synty characters face the
    other way, so everything is turned 180 degrees about Z. The skeleton stores
    each joint in armature space rather than relative to its parent, and the
    animation exporter reconstructs whole chains, so the rotation has to reach
    every joint; applying it at object level does that in one step.
    """
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    for obj in objects:
        if obj.parent is None:
            obj.rotation_euler.z += math.pi
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
    bpy.ops.object.select_all(action="DESELECT")


def _select_only(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def main():
    _install_addon_path()
    import importlib
    for name in _ADDON_MODULES:
        importlib.import_module(name).register()

    options = _parse_args(sys.argv)
    mesh_dir = os.path.join(options["out"], "meshes")
    skeleton_dir = os.path.join(options["out"], "skeletons")
    os.makedirs(mesh_dir, exist_ok=True)
    os.makedirs(skeleton_dir, exist_ok=True)
    extension = ".txt" if options["text"] else ".msh"

    wanted = set(options["parts"])
    sources = {}
    for folder in options["in"]:
        for entry in sorted(os.listdir(folder)):
            if not entry.lower().endswith(".fbx"):
                continue
            name = os.path.splitext(entry)[0]
            if wanted and name not in wanted:
                continue
            sources.setdefault(name, os.path.join(folder, entry))
    if not sources:
        raise SystemExit("No matching .fbx files under {}".format(", ".join(options["in"])))
    missing = sorted(wanted - set(sources))

    skeleton_written = os.path.exists(os.path.join(skeleton_dir, "skeleton.skl"))
    exported = 0
    failed = []
    for name in sorted(sources):
        try:
            bpy.ops.wm.read_factory_settings(use_empty=True)
            bpy.ops.import_scene.fbx(filepath=sources[name])
            normalize_rig([o for o in bpy.data.objects])
            mesh = _largest_mesh()
            if mesh is None:
                failed.append((name, "no mesh"))
                continue
            if options["rest_from"]:
                rig = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
                if rig is not None:
                    _rebind_to_rest(rig, mesh, options["rest_from"])
            _select_only(mesh)
            bpy.ops.export_mesh.msh(filepath=os.path.join(mesh_dir, name + extension))
            exported += 1

            if not skeleton_written:
                armature = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
                if armature is not None:
                    _select_only(armature)
                    bpy.ops.export_skeleton.skl(
                        filepath=os.path.join(skeleton_dir, "skeleton" + (".txt" if options["text"] else ".skl")))
                    skeleton_written = True
        except Exception as error:  # noqa: BLE001 - report and continue the batch
            failed.append((name, str(error)))

    print("=== sidekick_batch: {} meshes exported, {} failed ===".format(exported, len(failed)))
    for name, reason in failed:
        print("  FAILED {}: {}".format(name, reason))
    for name in missing:
        print("  MISSING {}: no .fbx found in any input folder".format(name))
    if failed or missing:
        raise SystemExit(1)


if __name__ == "__main__":
    main()

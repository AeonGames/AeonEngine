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
    options = {"in": [], "out": None, "parts": [], "text": False}
    index = 0
    while index < len(argv):
        argument = argv[index]
        if argument in ("--in", "-i"):
            index += 1
            options["in"].append(argv[index])
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
            "[--recipe <file.sk>] [--parts NAME ...] [--text]")
    return options


def _largest_mesh():
    """Some Synty parts carry a stray loose-edge mesh next to the real one."""
    meshes = [o for o in bpy.data.objects if o.type == "MESH" and len(o.data.polygons)]
    if not meshes:
        return None
    return max(meshes, key=lambda o: len(o.data.vertices))


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
            mesh = _largest_mesh()
            if mesh is None:
                failed.append((name, "no mesh"))
                continue
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

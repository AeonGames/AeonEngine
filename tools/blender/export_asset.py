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
"""Cook the currently open .blend into engine assets.

Drives the io_model_mdl exporter, which writes the model plus its meshes,
skeleton, animations, materials and textures. Textures packed into the .blend
are written out alongside the ones that live loose on disk, so materials
resolve either way.

Run headless from the repository root:

    blender -b --factory-startup -noaudio assets/aerin/aerin.blend \\
        --python tools/blender/export_asset.py -- \\
        --out game/aerin --prefix aerin --name aerin --venv <venv dir>

The CMake add_blender_asset() function is the intended caller.
"""

import glob
import os
import sys
import traceback

import bpy

# io_model_mdl checks these through addon_utils and silently skips whole asset
# categories when they are not enabled, so all of them are enabled up front.
_ADDON_MODULES = (
    "io_mesh_msh",
    "io_skeleton_skl",
    "io_animation_anm",
    "io_material_mtl",
    "io_model_mdl",
)


def _parse_args(argv):
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    options = {"out": None, "prefix": "", "name": None, "venv": None,
               "text": False, "force": True}
    index = 0
    while index < len(argv):
        argument = argv[index]
        if argument in ("--out", "-o"):
            index += 1
            options["out"] = argv[index]
        elif argument in ("--prefix", "-p"):
            index += 1
            options["prefix"] = argv[index]
        elif argument in ("--name", "-n"):
            index += 1
            options["name"] = argv[index]
        elif argument == "--venv":
            index += 1
            options["venv"] = argv[index]
        elif argument == "--text":
            options["text"] = True
        elif argument == "--no-force":
            options["force"] = False
        else:
            raise SystemExit("Unknown option {}".format(argument))
        index += 1
    if not options["out"]:
        raise SystemExit(
            "Usage: --out <directory> --prefix <resource prefix> "
            "[--name <model name>] [--venv <directory>] [--text] [--no-force]")
    if not options["name"]:
        options["name"] = os.path.splitext(
            os.path.basename(bpy.data.filepath))[0] or "model"
    return options


def _extend_sys_path(venv):
    """Make the exporters and the protobuf runtime importable.

    The exporters import the generated *_pb2 modules by bare name, and those in
    turn need a protobuf runtime Blender does not bundle; the venv built by the
    blender-python-venv target supplies it.
    """
    root = os.path.dirname(os.path.abspath(__file__))
    directories = [os.path.join(root, "modules"), os.path.join(root, "addons")]
    if venv:
        directories.extend(glob.glob(os.path.join(venv, "Lib", "site-packages")))
        directories.extend(
            glob.glob(os.path.join(venv, "lib", "python*", "site-packages")))
    for directory in directories:
        if directory not in sys.path:
            sys.path.insert(0, directory)
    try:
        import google.protobuf  # noqa: F401
    except ImportError:
        raise SystemExit(
            "The protobuf runtime is not importable. Build the "
            "blender-python-venv target, or pass --venv <directory>.")


def _enable_addons():
    """Enable the exporter add-ons from the repository's script directory.

    Registering the modules by hand is not enough: io_model_mdl gates on
    addon_utils.check(), which only reports add-ons Blender itself enabled.
    """
    scripts = os.path.dirname(os.path.abspath(__file__))
    directories = bpy.context.preferences.filepaths.script_directories
    if not any(os.path.normcase(os.path.abspath(entry.directory)) ==
               os.path.normcase(scripts) for entry in directories):
        entry = directories.new()
        entry.name = "AeonEngine"
        entry.directory = scripts
    bpy.utils.refresh_script_paths()
    bpy.ops.preferences.addon_refresh()

    import addon_utils
    for module in _ADDON_MODULES:
        bpy.ops.preferences.addon_enable(module=module)
        enabled, loaded = addon_utils.check(module)
        if not (enabled and loaded):
            raise SystemExit(
                "Failed to enable the {} add-on; the exporters would silently "
                "skip part of the asset.".format(module))


def main():
    options = _parse_args(sys.argv)
    _extend_sys_path(options["venv"])
    _enable_addons()

    directory = os.path.abspath(options["out"]) + os.sep
    os.makedirs(directory, exist_ok=True)
    # io_model_mdl names the model file after the scene.
    bpy.context.scene.name = options["name"]

    bpy.ops.export_model.mdl(
        'EXEC_DEFAULT',
        directory=directory,
        resource_prefix=options["prefix"],
        as_text=options["text"],
        force=options["force"],
        export_meshes=True,
        export_skeleton=True,
        export_animations=True,
        export_textures=True,
        export_materials=True)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        # Blender exits 0 on an uncaught script exception, which would let a
        # failed cook pass for a successful build.
        traceback.print_exc()
        sys.exit(1)

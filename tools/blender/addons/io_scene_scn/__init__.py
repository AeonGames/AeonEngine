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

bl_info = {
    "name": "AeonGames Scene Format (.scn)",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (2, 80, 0),
    "location": "File > Export > Export AeonGames Scene",
    "description": "Exports a scene to an AeonGames Scene (SCN) file, exporting one model per unique mesh and instancing duplicates, plus light and camera nodes",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export",
    "support": "COMMUNITY"
}

import bpy
from . import export


# Property used to remember the game root for the current Blender session.
# WindowManager datablocks are not saved to disk, so the value survives
# multiple exports in one session but resets when Blender restarts. The
# scene exporter writes the chosen game root here so any lower-layer exporter
# run afterwards in the same session can pick it up as its default.
_SESSION_GAME_ROOT = "aeon_game_root"
# Property stored on the Scene, so it is saved inside the .blend file and acts
# as a per-project override of the global preference.
_SCENE_GAME_ROOT = "aeon_game_root"
# Property stored on each Object, tagging what an empty marks. Kept in sync
# with _MARKER_TYPE_PROP in export.py.
_OBJECT_MARKER_TYPE = "aeon_marker_type"
# Property stored on each Object, naming which camera component a camera
# exports as. Kept in sync with _CAMERA_COMPONENT_PROP in export.py.
_OBJECT_CAMERA_COMPONENT = "aeon_camera_component"


class AeonScnAddonPreferences(bpy.types.AddonPreferences):
    '''Global, per-user defaults shared across this machine's .blend files.'''
    bl_idname = __package__

    game_root: bpy.props.StringProperty(
        name="Game Root",
        subtype="DIR_PATH",
        description="Default root directory assets are exported under. Used "
                    "as the export root unless overridden per .blend file or "
                    "for the current session",
        default=""
    )

    def draw(self, context):
        self.layout.prop(self, "game_root")


def scn_menu_func(self, context):
    self.layout.operator(
        export.SCN_OT_exporter.bl_idname,
        text="AeonGames Scene (.scn)")


def register():
    bpy.utils.register_class(AeonScnAddonPreferences)
    bpy.utils.register_class(export.SCN_OT_exporter)
    # Per-project override, saved inside the .blend file.
    if not hasattr(bpy.types.Scene, _SCENE_GAME_ROOT):
        bpy.types.Scene.aeon_game_root = bpy.props.StringProperty(
            name="Game Root",
            subtype="DIR_PATH",
            description="Per-project export root override, saved in this "
                        ".blend file",
            default=""
        )
    # Per-project scene-name override (output folder + scene file name),
    # saved inside the .blend file.
    if not hasattr(bpy.types.Scene, "aeon_export_name"):
        bpy.types.Scene.aeon_export_name = bpy.props.StringProperty(
            name="Scene Name",
            description="Per-project override for the exported scene name, "
                        "saved in this .blend file",
            default=""
        )
    # Session-sticky value, not saved to disk.
    if not hasattr(bpy.types.WindowManager, _SESSION_GAME_ROOT):
        bpy.types.WindowManager.aeon_game_root = bpy.props.StringProperty(
            name="Session Game Root",
            subtype="DIR_PATH",
            default=""
        )
    # Kind of marker an empty stands for, exported as the Marker component's
    # Type property.
    if not hasattr(bpy.types.Object, _OBJECT_MARKER_TYPE):
        bpy.types.Object.aeon_marker_type = bpy.props.StringProperty(
            name="Marker Type",
            description="Kind of marker this empty stands for, e.g. Spawn or "
                        "Attach. Exported as the Marker component's Type",
            default=""
        )
    # Which camera component a camera object exports as. Only the components
    # driven purely by field of view and clipping planes are offered; the
    # others need values with no Blender equivalent.
    if not hasattr(bpy.types.Object, _OBJECT_CAMERA_COMPONENT):
        bpy.types.Object.aeon_camera_component = bpy.props.EnumProperty(
            name="Camera Component",
            description="Engine component this camera exports as",
            items=[
                ("Camera", "Camera", "Plain camera fixed to its node"),
                ("Free Camera", "Free Camera", "Free-fly debug camera"),
            ],
            default="Camera"
        )
    bpy.types.TOPBAR_MT_file_export.append(scn_menu_func)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(scn_menu_func)
    if hasattr(bpy.types.Object, _OBJECT_CAMERA_COMPONENT):
        del bpy.types.Object.aeon_camera_component
    if hasattr(bpy.types.Object, _OBJECT_MARKER_TYPE):
        del bpy.types.Object.aeon_marker_type
    if hasattr(bpy.types.WindowManager, _SESSION_GAME_ROOT):
        del bpy.types.WindowManager.aeon_game_root
    if hasattr(bpy.types.Scene, "aeon_export_name"):
        del bpy.types.Scene.aeon_export_name
    if hasattr(bpy.types.Scene, _SCENE_GAME_ROOT):
        del bpy.types.Scene.aeon_game_root
    bpy.utils.unregister_class(export.SCN_OT_exporter)
    bpy.utils.unregister_class(AeonScnAddonPreferences)


if __name__ == "__main__":
    register()

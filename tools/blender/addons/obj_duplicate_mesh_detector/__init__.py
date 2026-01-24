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
    "name": "Duplicate Mesh Detector",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (5, 0, 0),
    "location": "View3D > Sidebar > Tools > Duplicate Mesh Detector",
    "description": "Detects duplicate meshes based on point cloud comparison",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"}

import bpy
from . import detector


class DUPLICATEMESH_PT_panel(bpy.types.Panel):
    """Panel for Duplicate Mesh Detector"""
    bl_label = "Duplicate Mesh Detector"
    bl_idname = "DUPLICATEMESH_PT_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Tool'

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.prop(scene, "duplicate_mesh_tolerance")
        layout.prop(scene, "duplicate_mesh_use_world_space")
        layout.prop(scene, "duplicate_mesh_compare_uvs")
        layout.prop(scene, "duplicate_mesh_selected_only")
        layout.operator(detector.DUPLICATEMESH_OT_detect.bl_idname)

        # Display results if available
        if hasattr(context.scene, "duplicate_mesh_results") and context.scene.duplicate_mesh_results:
            box = layout.box()
            box.label(text="Duplicate Groups Found:")
            for group in context.scene.duplicate_mesh_results.split("|"):
                if group:
                    box.label(text=group)
            
            # Add convert to linked button
            layout.operator(detector.DUPLICATEMESH_OT_convert_linked.bl_idname)


def register():
    bpy.utils.register_class(detector.DUPLICATEMESH_OT_detect)
    bpy.utils.register_class(detector.DUPLICATEMESH_OT_convert_linked)
    bpy.utils.register_class(DUPLICATEMESH_PT_panel)

    bpy.types.Scene.duplicate_mesh_tolerance = bpy.props.FloatProperty(
        name="Tolerance",
        description="Distance tolerance for considering vertices as matching",
        default=0.0001,
        min=0.0,
        max=1.0,
        precision=6
    )
    bpy.types.Scene.duplicate_mesh_use_world_space = bpy.props.BoolProperty(
        name="Use World Space",
        description="Compare vertices in world space (includes object transforms)",
        default=False
    )
    bpy.types.Scene.duplicate_mesh_compare_uvs = bpy.props.BoolProperty(
        name="Compare UV Maps",
        description="Also compare UV maps when detecting duplicates",
        default=False
    )
    bpy.types.Scene.duplicate_mesh_selected_only = bpy.props.BoolProperty(
        name="Selected Only",
        description="Only compare selected mesh objects",
        default=False
    )
    bpy.types.Scene.duplicate_mesh_results = bpy.props.StringProperty(
        name="Results",
        description="Detection results",
        default=""
    )


def unregister():
    bpy.utils.unregister_class(DUPLICATEMESH_PT_panel)
    bpy.utils.unregister_class(detector.DUPLICATEMESH_OT_convert_linked)
    bpy.utils.unregister_class(detector.DUPLICATEMESH_OT_detect)

    del bpy.types.Scene.duplicate_mesh_tolerance
    del bpy.types.Scene.duplicate_mesh_use_world_space
    del bpy.types.Scene.duplicate_mesh_compare_uvs
    del bpy.types.Scene.duplicate_mesh_selected_only
    del bpy.types.Scene.duplicate_mesh_results


if __name__ == "__main__":
    register()

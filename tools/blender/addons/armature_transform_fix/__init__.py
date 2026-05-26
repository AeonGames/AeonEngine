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

import bpy
from . import transform_fix

bl_info = {
    "name": "AeonGames Armature Transform Fix",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (5, 0, 0),
    "location": "View3D > Sidebar > Tool > Armature Transform Fix",
    "description": "Apply armature rotation/scale and rescale pose-bone location keys so actions do not drift",
    "warning": "Designed for rigs whose action translation lives in pose-bone location curves",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Animation",
    "support": "COMMUNITY",
}


def armature_fix_menu_func(self, _context):
    self.layout.operator(
        transform_fix.ARMATUREFIX_OT_apply_transform_fix.bl_idname,
        text="Apply Armature Transform Fix")


def register():
    bpy.utils.register_class(transform_fix.ARMATUREFIX_OT_apply_transform_fix)
    bpy.utils.register_class(transform_fix.ARMATUREFIX_PT_panel)
    bpy.types.VIEW3D_MT_object.append(armature_fix_menu_func)


def unregister():
    bpy.types.VIEW3D_MT_object.remove(armature_fix_menu_func)
    bpy.utils.unregister_class(transform_fix.ARMATUREFIX_PT_panel)
    bpy.utils.unregister_class(transform_fix.ARMATUREFIX_OT_apply_transform_fix)


if __name__ == "__main__":
    register()
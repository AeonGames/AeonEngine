# Copyright (C) 2016,2017,2019 Rodrigo Jose Hernandez Cordoba
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

from . import export
import bpy
bl_info = {
    "name": "AeonGames Geometry Format (.gty)",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (2, 7, 0),
    "location": "File > Export > Export AeonGames Geometry",
    "description": "Exports a mesh to an AeonGames Geometry (GTY) file",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}


def gty_menu_func(self, context):
    self.layout.operator(
        export.GTYExporter.bl_idname,
        text="AeonGames Collision Geometry (.gty)")


def register():
    bpy.utils.register_class(export.GTYExporter)
    bpy.types.INFO_MT_file_export.append(gty_menu_func)


def unregister():
    bpy.utils.unregister_class(export.GTYExporter)
    bpy.types.INFO_MT_file_export.remove(gty_menu_func)


if __name__ == "__main__":
    register()

# Copyright 2016 Rodrigo Jose Hernandez Cordoba
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
    "name": "AeonGames Mesh Format (.msh)",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (2, 7, 0),
    "location": "File > Export > Export AeonGames Mesh",
    "description": "Exports a mesh to an AeonGames Mesh (MSH) file",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

import bpy
from . import export


def msh_menu_func(self, context):
    self.layout.operator(
        export.MSHExporter.bl_idname,
        text="AeonGames Mesh (.msh)")
    # bpy.types.INFO_MT_file_export.remove(msh_menu_func)


def register():
    bpy.utils.register_class(export.MSHExporter)
    bpy.types.INFO_MT_file_export.append(msh_menu_func)

if __name__ == "__main__":
    register()

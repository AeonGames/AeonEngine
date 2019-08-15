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
    "name": "AeonGames Collision Format (.cln)",
    "author": "Rodrigo Hernandez",
    "version": (1, 0, 0),
    "blender": (2, 80, 0),
    "location": "File > Export > Export AeonGames Collision Data",
    "description": "Exports a mesh as AeonGames Collision Data file (CLN)",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}


def cln_menu_func(self, context):
    self.layout.operator(
        export.CLN_OT_exporter.bl_idname,
        text="AeonGames Collision Data (.cln)")


def register():
    bpy.utils.register_class(export.CLN_OT_exporter)
    bpy.types.TOPBAR_MT_file_export.append(cln_menu_func)


def unregister():
    bpy.utils.unregister_class(export.CLN_OT_exporter)
    bpy.types.TOPBAR_MT_file_export.remove(cln_menu_func)


if __name__ == "__main__":
    register()

# Copyright (C) 2017,2026 Rodrigo Jose Hernandez Cordoba
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
    "name": "Import Unreal Skeleton Mesh (.psk/.pskx)/Animation Set (.psa)",
    "author": "Rodrigo Hernandez",
    "version": (2, 0, 0),
    "blender": (4, 2, 0),
    "location": "File > Import > Skeleton Mesh (.psk/.pskx)/Animation Set (.psa)",
    "description": "Import Unreal Skeleton Mesh (UE1-5) and Animation Data",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export",
    "support": "COMMUNITY"
}

import bpy
from bpy.props import CollectionProperty, IntProperty, StringProperty, BoolProperty, FloatProperty
from . import import_psk_psa


def psk_menu_func(self, context):
    self.layout.operator(
        import_psk_psa.IMPORT_OT_psk.bl_idname,
        text="Unreal Skeleton Mesh (.psk/.pskx)")


def psa_menu_func(self, context):
    self.layout.operator(
        import_psk_psa.IMPORT_OT_psa.bl_idname,
        text="Unreal Animation Set (.psa)")


def register():
    bpy.utils.register_class(import_psk_psa.UDKImportArmaturePG)
    bpy.utils.register_class(import_psk_psa.IMPORT_OT_psk)
    bpy.utils.register_class(import_psk_psa.IMPORT_OT_psa)
    bpy.utils.register_class(import_psk_psa.Panel_UDKImport)
    bpy.utils.register_class(import_psk_psa.OBJECT_OT_PSKPath)
    bpy.utils.register_class(import_psk_psa.OBJECT_OT_PSAPath)
    bpy.utils.register_class(import_psk_psa.OBJECT_OT_UDKImportArmature)
    bpy.types.Scene.udkimportarmature_list = CollectionProperty(type=import_psk_psa.UDKImportArmaturePG)
    bpy.types.Scene.udkimportarmature_list_idx = IntProperty()
    bpy.types.Scene.udk_importpsk = StringProperty(
        name="Import .psk",
        description="Skeleton mesh file path for psk",
        default="")
    bpy.types.Scene.udk_importpsa = StringProperty(
        name="Import .psa",
        description="Animation Data to Action Set(s) file path for psa",
        default="")
    bpy.types.Scene.udk_importarmatureselect = BoolProperty(
        name="Armature Selected",
        description="Select Armature to Import psa animation data",
        default=False)
    bpy.types.Scene.unrealbonesize = FloatProperty(
        name="Bone Length",
        description="Bone Length from head to tail distance",
        default=1, min=0.001, max=1000)
    bpy.types.TOPBAR_MT_file_import.append(psk_menu_func)
    bpy.types.TOPBAR_MT_file_import.append(psa_menu_func)


def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(psk_menu_func)
    bpy.types.TOPBAR_MT_file_import.remove(psa_menu_func)
    del bpy.types.Scene.unrealbonesize
    del bpy.types.Scene.udk_importarmatureselect
    del bpy.types.Scene.udk_importpsa
    del bpy.types.Scene.udk_importpsk
    del bpy.types.Scene.udkimportarmature_list
    del bpy.types.Scene.udkimportarmature_list_idx
    bpy.utils.unregister_class(import_psk_psa.OBJECT_OT_UDKImportArmature)
    bpy.utils.unregister_class(import_psk_psa.OBJECT_OT_PSAPath)
    bpy.utils.unregister_class(import_psk_psa.OBJECT_OT_PSKPath)
    bpy.utils.unregister_class(import_psk_psa.Panel_UDKImport)
    bpy.utils.unregister_class(import_psk_psa.IMPORT_OT_psa)
    bpy.utils.unregister_class(import_psk_psa.IMPORT_OT_psk)
    bpy.utils.unregister_class(import_psk_psa.UDKImportArmaturePG)


if __name__ == "__main__":
    register()

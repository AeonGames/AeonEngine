# Copyright (C) 2016,2017,2019,2026 Rodrigo Jose Hernandez Cordoba
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
from . import armature_lossless_connect
import bpy

bl_info = {
    "name": "Lossless Bone Connect",
    "author": "Rodrigo Hernandez",
    "version": (1, 0),
    "blender": (2, 80, 0),
    "location": "Object > Connect Bones",
    "description": "Connects all single child bones to their parent tail without moving their heads",
    "category": "Object",
}

def register():
    bpy.utils.register_class(armature_lossless_connect.LosslessConnectOperator)
    bpy.utils.register_class(armature_lossless_connect.LosslessConnectMenu)


def unregister():
    bpy.utils.unregister_class(armature_lossless_connect.LosslessConnectMenu)
    bpy.utils.unregister_class(armature_lossless_connect.LosslessConnectOperator)


if __name__ == "__main__":
    register()

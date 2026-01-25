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

import bpy
import mathutils
import math


def main(context):
    if (context.active_object.type != 'ARMATURE'):
        return {'CANCELLED'}
    bpy.ops.object.mode_set(mode='EDIT')
    armature = context.active_object.data
    pose = context.active_object.pose

    for key, value in armature.bones.items():
        print("Edit", value.matrix)
        print("Pose", pose.bones[value.name].matrix)
    print("-----------------------------------------------")
    for key, value in armature.edit_bones.items():
        if len(value.children) == 1 and not value.children[0].use_connect and (
                value.head - value.children[0].head).length >= 0.0001:
            before = value.matrix
            value.tail = value.children[0].head
            value.children[0].use_connect = True
            after = value.matrix
            before.invert()
            difference = before * after

            for action in bpy.data.actions:
                print(
                    len(action.groups[value.name].channels[0].keyframe_points))
                print(
                    len(action.groups[value.name].channels[1].keyframe_points))
                print(
                    len(action.groups[value.name].channels[2].keyframe_points))
                print(
                    len(action.groups[value.name].channels[3].keyframe_points))
                print(
                    len(action.groups[value.name].channels[4].keyframe_points))
                print(
                    len(action.groups[value.name].channels[5].keyframe_points))
                print(
                    len(action.groups[value.name].channels[6].keyframe_points))
                print(
                    len(action.groups[value.name].channels[7].keyframe_points))
                print(
                    len(action.groups[value.name].channels[8].keyframe_points))
                print(
                    len(action.groups[value.name].channels[9].keyframe_points))
                # loc = mathutils.Vector(action.groups[value.name].channels[0].keyframe_points.co[1],
                # action.groups[value.name].channels[1].keyframe_points.co[1],
                # action.groups[value.name].channels[2].keyframe_points.co[1])
                # rot = mathutils.Quaternion(action.groups[value.name].channels[3].keyframe_points.co[1],
                # action.groups[value.name].channels[4].keyframe_points.co[1],
                # action.groups[value.name].channels[5].keyframe_points.co[1],
                # action.groups[value.name].channels[6].keyframe_points.co[1])
                # sca = mathutils.Vector(action.groups[value.name].channels[7].keyframe_points.co[1],
                # action.groups[value.name].channels[0].keyframe_points.co[1],
                # action.groups[value.name].channels[9].keyframe_points.co[1])
                # print(loc,rot,sca)
    bpy.ops.object.mode_set(mode='EDIT', toggle=True)


class LosslessConnectOperator(bpy.types.Operator):
    """Connect bones without lossing the head position"""
    bl_idname = "object.lossless_connect_operator"
    bl_label = "Lossless Bone Connect Operator"

    @classmethod
    def poll(cls, context):
        return context.active_object.type == 'ARMATURE'

    def execute(self, context):
        main(context)
        return {'FINISHED'}


class LosslessConnectMenu(bpy.types.Menu):
    bl_label = "Connect"
    bl_idname = "OBJECT_MT_lossless_connect_menu"

    def draw(self, context):
        layout = self.layout
        layout.operator(
            LosslessConnectOperator.bl_idname,
            text="Lossless Connect")

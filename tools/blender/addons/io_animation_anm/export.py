# Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba
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
import struct
import mathutils
import math
import animation_pb2
import google.protobuf.text_format
from multiprocessing.dummy import Pool as ThreadPool, Lock as ThreadLock


class Bone():

    def __init__(self, action, bone):
        self.action = action
        self.bone = bone
        self.scale = (
            action.fcurves.find("pose.bones[\"" + bone.name + "\"].scale", 0),
            action.fcurves.find("pose.bones[\"" + bone.name + "\"].scale", 1),
            action.fcurves.find("pose.bones[\"" + bone.name + "\"].scale", 2)
        )
        self.rotation = (
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                0),
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                1),
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                2),
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                3))
        self.translation = (
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].location",
                0),
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].location",
                1),
            action.fcurves.find(
                "pose.bones[\"" + bone.name + "\"].location",
                2))

    def get_matrix(self, frame):
        # Sadly, mathutils.Matrix.Scale does not work the way we're forced to
        # do it here.
        scale = mathutils.Matrix((
            (1.0 if self.scale[0] is None else self.scale[0].evaluate(float(frame)), 0, 0, 0),
            (0, 1.0 if self.scale[1] is None else self.scale[1].evaluate(float(frame)), 0, 0),
            (0, 0, 1.0 if self.scale[2] is None else self.scale[2].evaluate(float(frame)), 0),
            (0, 0, 0, 1.0)))
        rotation = mathutils.Quaternion(
            (1.0 if self.rotation[0] is None else self.rotation[0].evaluate(
                float(frame)), 0.0 if self.rotation[1] is None else self.rotation[1].evaluate(
                float(frame)), 0.0 if self.rotation[2] is None else self.rotation[2].evaluate(
                float(frame)), 0.0 if self.rotation[3] is None else self.rotation[3].evaluate(
                    float(frame)))).to_matrix().to_4x4()
        translation = mathutils.Matrix.Translation(
            (0.0 if self.translation[0] is None else self.translation[0].evaluate(
                float(frame)), 0.0 if self.translation[1] is None else self.translation[1].evaluate(
                float(frame)), 0.0 if self.translation[2] is None else self.translation[2].evaluate(
                float(frame))))
        if(self.bone.parent):
            return self.bone.parent.matrix_local.inverted() * self.bone.matrix_local * \
                translation * rotation * scale
        return self.bone.matrix_local * translation * rotation * scale

    def __str__(self):
        return "Bone: " + self.bone.name + "\n" + \
            "Parent: " + (self.bone.parent.name if self.bone.parent is not None else "None") + "\n" + \
            "Rotation:\n" + str(self.rotation[0].data_path) + "\n" + \
            "Translation:\n" + str(self.translation[0].data_path) + "\n"


class ANMExporter(bpy.types.Operator):
    '''Exports an armature's actions to AeonGames animation (ANM) file'''
    bl_idname = "export_armature.anm"
    bl_label = "Export AeonGames Animations"

    directory = bpy.props.StringProperty(subtype='DIR_PATH')

    EPSILON = 0.00001

    @classmethod
    def poll(cls, context):
        if (context.active_object.type ==
                'ARMATURE') and context.active_object.animation_data:
            return True
        return False

    def execute(self, context):
        if (context.active_object.type !=
                'ARMATURE') and not context.active_object.animation_data:
            return {'CANCELLED'}
        bpy.ops.object.mode_set()
        current_action = context.active_object.animation_data.action
        current_frame = context.scene.frame_current

        for action in bpy.data.actions:
            # Create Protocol Buffer
            animation_buffer = animation_pb2.AnimationBuffer()
            # Initialize Protocol Buffer Message
            animation_buffer.Version = 1
            animation_buffer.FrameRate = context.scene.render.fps
            animation_buffer.Duration = (1.0 / context.scene.render.fps) * (
                (int(action.frame_range[1]) - int(action.frame_range[0])) + 1)
            print(action.name)
            bones = []
            for bone in context.active_object.data.bones:
                bones.append(Bone(action, bone))

            for frame in range(int(action.frame_range[0]), int(
                    action.frame_range[1]) + 1):
                frame_buffer = animation_buffer.Frame.add()
                for bone in bones:
                    translation, rotation, scale = bone.get_matrix(
                        frame).decompose()
                    rotation.normalize()
                    bone_buffer = frame_buffer.Bone.add()
                    bone_buffer.Index = context.active_object.data.bones.find(
                        bone.bone.name)
                    bone_buffer.Scale.x, bone_buffer.Scale.y, bone_buffer.Scale.z = scale
                    bone_buffer.Rotation.w, bone_buffer.Rotation.x, bone_buffer.Rotation.y, bone_buffer.Rotation.z = rotation
                    bone_buffer.Translation.x, bone_buffer.Translation.y, bone_buffer.Translation.z = translation

            print("Writting", self.directory + "/" + action.name + ".anm", ".")
            out = open(self.directory + "/" + action.name + ".anm", "wb")
            magick_struct = struct.Struct('8s')
            out.write(magick_struct.pack(b'AEONANM\x00'))
            out.write(animation_buffer.SerializeToString())
            out.close()
            print("Done.")
            print(
                "Writting",
                self.directory +
                "/" +
                action.name +
                ".anm.txt",
                ".")
            out = open(self.directory + "/" + action.name + ".anm.txt", "wt")
            out.write("AEONANM\n")
            out.write(
                google.protobuf.text_format.MessageToString(animation_buffer))
            out.close()
        current_action = context.active_object.animation_data.action
        current_frame = context.scene.frame_current
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

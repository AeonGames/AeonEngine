# Copyright (C) 2017,2019,2021 Rodrigo Jose Hernandez Cordoba
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
import os
import struct
import skeleton_pb2
import google.protobuf.text_format


class SKL_OT_exporter(bpy.types.Operator):

    '''Exports an armature to an AeonGames Skeleton (SKL) file'''
    bl_idname = "export_skeleton.skl"
    bl_label = "Export AeonGames Skeleton"

    filepath: bpy.props.StringProperty(subtype='FILE_PATH')

    @classmethod
    def poll(cls, context):
        if (context.active_object.type == 'ARMATURE'):
            return True
        return False

    def execute(self, context):
        if (context.active_object.type != 'ARMATURE'):
            return {'CANCELLED'}
        bpy.ops.object.mode_set()
        self.filepath = bpy.path.ensure_ext(self.filepath, ".skl")
        # Create Protocol Buffer
        skeleton_buffer = skeleton_pb2.SkeletonMsg()
        # Initialize Protocol Buffer Message
        skeleton_buffer.Version = 1

        armature = context.active_object.data
        for bone in armature.bones:
            joint = skeleton_buffer.Joint.add()
            joint.ParentIndex = -1
            if bone.parent is not None:
                joint.ParentIndex = armature.bones.find(bone.parent.name)
            # If the following assert is hit, the order of bones should be
            # changed.
            assert armature.bones.find(bone.name) > joint.ParentIndex
            # print(bone.name,"index:",armature.bones.find(bone.name),"parent index:", parent_index)
            # What is refered as the matrix_local is really the bone matrix in the armature model space.
            # Note: skeleton bones are relative to the armature origin, not
            # their parents, unlike animation bones.
            translation, rotation, scale = bone.matrix_local.decompose()
            inv_translation, inv_rotation, inv_scale = bone.matrix_local.inverted(
            ).decompose()
            rotation.normalize()
            inv_rotation.normalize()
            print("Index", armature.bones.find(bone.name))
            print("Translation", translation)
            print("Rotation", rotation)
            print("Scale", scale)
            print("INV Translation", inv_translation)
            print("INV Rotation", inv_rotation)
            print("INV Scale", inv_scale)
            joint.Scale.x, joint.Scale.y, joint.Scale.z = scale
            joint.Rotation.w, joint.Rotation.x, joint.Rotation.y, joint.Rotation.z = rotation
            joint.Translation.x, joint.Translation.y, joint.Translation.z = translation
            joint.InvertedScale.x, joint.InvertedScale.y, joint.InvertedScale.z = inv_scale
            joint.InvertedRotation.w, joint.InvertedRotation.x, joint.InvertedRotation.y, joint.InvertedRotation.z = inv_rotation
            joint.InvertedTranslation.x, joint.InvertedTranslation.y, joint.InvertedTranslation.z = inv_translation
            joint.Name = bone.name

        # Open File for Writing
        print("Writting", self.filepath, ".")
        out = open(self.filepath, "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONSKL\x00'))
        out.write(skeleton_buffer.SerializeToString())
        out.close()
        print("Done.")
        print("Writting", self.filepath + ".txt", ".")
        out = open(self.filepath + ".txt", "wt")
        out.write("AEONSKL\n")
        out.write(google.protobuf.text_format.MessageToString(skeleton_buffer))
        out.close()
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        if not self.filepath:
            self.filepath = bpy.path.ensure_ext(
                os.path.dirname(
                    bpy.data.filepath) +
                os.sep +
                context.scene.name,
                ".skl")
        else:
            self.filepath = bpy.path.ensure_ext(self.filepath, ".skl")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

# Copyright (C) 2017,2019,2021,2026 Rodrigo Jose Hernandez Cordoba
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
import mathutils
import animation_pb2
import google.protobuf.text_format


def _get_action_fcurves(action, slot=None):
    """Return the FCurves collection for an Action.

    Handles both the legacy (pre-Blender 4.4) Action.fcurves API and the
    slotted-actions API introduced in 4.4 / required in 5.x, where fcurves
    live on ``action.layers[i].strips[j].channelbag(slot).fcurves``.

    If ``slot`` is None, the first slot that has a channelbag is used.
    Returns ``None`` if no fcurves can be found.
    """
    # Legacy path: some Blender versions still expose Action.fcurves.
    legacy = getattr(action, "fcurves", None)
    if legacy is not None:
        return legacy
    # Slotted path.
    layers = getattr(action, "layers", None)
    if not layers:
        return None
    layer = layers[0]
    strips = getattr(layer, "strips", None)
    if not strips:
        return None
    strip = strips[0]
    candidate_slots = [slot] if slot is not None else list(getattr(action, "slots", []))
    for s in candidate_slots:
        if s is None:
            continue
        cb = strip.channelbag(s)
        if cb is not None:
            return cb.fcurves
    return None


class Bone():

    def __init__(self, fcurves, bone):
        self.fcurves = fcurves
        self.bone = bone
        self.scale = (
            fcurves.find("pose.bones[\"" + bone.name + "\"].scale", index=0),
            fcurves.find("pose.bones[\"" + bone.name + "\"].scale", index=1),
            fcurves.find("pose.bones[\"" + bone.name + "\"].scale", index=2)
        )
        self.rotation = (
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                index=0),
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                index=1),
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                index=2),
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].rotation_quaternion",
                index=3))
        self.translation = (
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].location",
                index=0),
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].location",
                index=1),
            fcurves.find(
                "pose.bones[\"" + bone.name + "\"].location",
                index=2))

    def get_matrix(self, frame):
        # Sadly, mathutils.Matrix.Scale does not work the way we're forced to
        # do it here.
        scale = mathutils.Matrix((
            (1.0 if self.scale[0] is None else self.scale[
             0].evaluate(float(frame)), 0, 0, 0),
            (0, 1.0 if self.scale[1] is None else self.scale[
             1].evaluate(float(frame)), 0, 0),
            (0, 0, 1.0 if self.scale[2] is None else self.scale[
             2].evaluate(float(frame)), 0),
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
        return translation @ rotation @ scale

    def __str__(self):
        return "Bone: " + self.bone.name + "\n" + \
            "Parent: " + (self.bone.parent.name if self.bone.parent is not None else "None") + "\n" + \
            "Rotation:\n" + str(self.rotation[0].data_path) + "\n" + \
            "Translation:\n" + str(self.translation[0].data_path) + "\n"


class ANM_OT_exporter(bpy.types.Operator):

    '''Exports an armature's actions to AeonGames animation (ANM) file'''
    bl_idname= "export_armature.anm"
    bl_label= "Export AeonGames Animations"

    directory: bpy.props.StringProperty(subtype='DIR_PATH')
    as_text: bpy.props.BoolProperty(
        name="Export as Text",
        description="Write the protobuf message as a human-readable text file (.txt) instead of the binary .anm",
        default=False
    )

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
        current_slot = getattr(context.active_object.animation_data, "action_slot", None)
        current_frame = context.scene.frame_current

        for action in bpy.data.actions:
            fcurves = _get_action_fcurves(action, current_slot if action is current_action else None)
            if fcurves is None:
                print("Skipping action", action.name, "(no fcurves / no usable slot)")
                continue
            # Create Protocol Buffer
            animation_buffer = animation_pb2.AnimationMsg()
            # Initialize Protocol Buffer Message
            animation_buffer.Version = 1
            animation_buffer.FrameRate = context.scene.render.fps
            animation_buffer.Duration = (1.0 / context.scene.render.fps) * (
                (int(action.frame_range[1]) - int(action.frame_range[0])) + 1)
            print(action.name)
            bones = []
            for bone in context.active_object.data.bones:
                bones.append(Bone(fcurves, bone))

            for frame in range(int(action.frame_range[0]), int(
                    action.frame_range[1]) + 1):
                # We have to reconstruct the matrix skeleton
                frame_matrices = []
                for bone in bones:
                    if not bone.bone.parent:
                        frame_matrices.append(
                            bone.bone.matrix_local @ bone.get_matrix(frame))
                    else:
                        frame_matrices.append(
                            frame_matrices[
                                context.active_object.data.bones.find(
                                    bone.bone.parent.name)] @
                            bone.bone.parent.matrix_local.inverted() @
                            bone.bone.matrix_local @
                            bone.get_matrix(frame))
                frame_buffer = animation_buffer.Frame.add()
                for frame_matrix in frame_matrices:
                    translation, rotation, scale = frame_matrix.decompose()
                    rotation.normalize()
                    bone_buffer = frame_buffer.Bone.add()
                    bone_buffer.Scale.x, bone_buffer.Scale.y, bone_buffer.Scale.z = scale
                    bone_buffer.Rotation.w, bone_buffer.Rotation.x, bone_buffer.Rotation.y, bone_buffer.Rotation.z = rotation
                    bone_buffer.Translation.x, bone_buffer.Translation.y, bone_buffer.Translation.z = translation

            anm_path = self.directory + os.sep + action.name + ".anm"
            if self.as_text:
                text_path = self.directory + os.sep + action.name + ".txt"
                print("Writting", text_path, ".")
                out = open(text_path, "wt")
                out.write("AEONANM\n")
                out.write(
                    google.protobuf.text_format.MessageToString(animation_buffer))
                out.close()
                print("Done.")
            else:
                print("Writting", anm_path, ".")
                out = open(anm_path, "wb")
                magick_struct = struct.Struct('8s')
                out.write(magick_struct.pack(b'AEONANM\x00'))
                out.write(animation_buffer.SerializeToString())
                out.close()
                print("Done.")
        current_action = context.active_object.animation_data.action
        current_frame = context.scene.frame_current
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

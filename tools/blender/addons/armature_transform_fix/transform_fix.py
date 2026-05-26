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

import re

import bpy


LOCATION_RE = re.compile(r'^pose\.bones\["[^"]+"\]\.location$')


def iter_action_fcurves(action):
    """Yield all FCurves belonging to an Action across legacy and slotted APIs."""
    legacy = getattr(action, "fcurves", None)
    if legacy is not None:
        yield from legacy
        return
    for layer in getattr(action, "layers", []):
        for strip in getattr(layer, "strips", []):
            for slot in getattr(action, "slots", []):
                channelbag = strip.channelbag(slot)
                if channelbag is None:
                    continue
                yield from channelbag.fcurves


def scale_location_fcurves(actions, factors):
    scaled_curves = 0
    scaled_keys = 0
    for action in actions:
        for fcurve in iter_action_fcurves(action):
            if not LOCATION_RE.match(fcurve.data_path):
                continue
            factor = factors[fcurve.array_index]
            for keyframe in fcurve.keyframe_points:
                keyframe.co.y *= factor
                keyframe.handle_left.y *= factor
                keyframe.handle_right.y *= factor
                scaled_keys += 1
            fcurve.update()
            scaled_curves += 1
    return scaled_curves, scaled_keys


class ARMATUREFIX_OT_apply_transform_fix(bpy.types.Operator):
    """Apply armature rotation/scale and fix pose-bone location keys"""
    bl_idname = "object.aeongames_apply_armature_transform_fix"
    bl_label = "Apply Armature Transform Fix"
    bl_options = {'REGISTER', 'UNDO'}

    apply_rotation: bpy.props.BoolProperty(
        name="Apply Rotation",
        description="Apply the active armature's object rotation to its rest pose",
        default=True,
    )
    apply_scale: bpy.props.BoolProperty(
        name="Apply Scale",
        description="Apply the active armature's object scale and rescale pose-bone location keys",
        default=True,
    )

    @classmethod
    def poll(cls, context):
        return (
            context.active_object is not None and
            context.active_object.type == 'ARMATURE'
        )

    def execute(self, context):
        armature_object = context.active_object
        scale_factors = tuple(armature_object.scale)

        if not self.apply_rotation and not self.apply_scale:
            self.report({'WARNING'}, "Nothing selected to apply")
            return {'CANCELLED'}

        if not self.apply_scale:
            scale_factors = (1.0, 1.0, 1.0)

        hidden_state = armature_object.hide_get()
        hide_viewport = armature_object.hide_viewport
        hide_select = armature_object.hide_select
        selected_objects = list(context.selected_objects)

        bpy.ops.object.mode_set(mode='OBJECT')
        armature_object.hide_set(False)
        armature_object.hide_viewport = False
        armature_object.hide_select = False
        bpy.ops.object.select_all(action='DESELECT')
        armature_object.select_set(True)
        context.view_layer.objects.active = armature_object

        result = bpy.ops.object.transform_apply(
            location=False,
            rotation=self.apply_rotation,
            scale=self.apply_scale)
        if result != {'FINISHED'}:
            self.report({'ERROR'}, f"transform_apply failed: {result}")
            return {'CANCELLED'}

        scaled_curves = 0
        scaled_keys = 0
        if self.apply_scale:
            scaled_curves, scaled_keys = scale_location_fcurves(
                bpy.data.actions,
                scale_factors)

        bpy.ops.object.select_all(action='DESELECT')
        for selected_object in selected_objects:
            if selected_object.name in bpy.data.objects:
                bpy.data.objects[selected_object.name].select_set(True)
        context.view_layer.objects.active = armature_object
        armature_object.hide_select = hide_select
        armature_object.hide_viewport = hide_viewport
        armature_object.hide_set(hidden_state)

        self.report(
            {'INFO'},
            f"Applied transform fix to '{armature_object.name}'; scaled {scaled_curves} curves and {scaled_keys} keys")
        return {'FINISHED'}


class ARMATUREFIX_PT_panel(bpy.types.Panel):
    bl_label = "Armature Transform Fix"
    bl_idname = "ARMATUREFIX_PT_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Tool'

    def draw(self, _context):
        layout = self.layout
        layout.label(text="Fix action drift after Apply Rotation/Scale")
        layout.operator(ARMATUREFIX_OT_apply_transform_fix.bl_idname)

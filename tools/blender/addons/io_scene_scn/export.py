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

import bpy
import os
import math
import struct
import addon_utils
import mathutils
import scene_pb2
import google.protobuf.text_format


# Name of the game root property mirrored on the WindowManager (session) and
# Scene (per-.blend). Kept in sync with the definitions in __init__.py.
_GAME_ROOT_PROP = "aeon_game_root"


def _get_addon_preferences(context):
    '''Return this addon's AddonPreferences, or None if unavailable.'''
    try:
        return context.preferences.addons[__package__].preferences
    except (KeyError, AttributeError):
        return None


def resolve_game_root(context):
    '''Resolve the default game root: session -> scene -> global preference.'''
    session = getattr(context.window_manager, _GAME_ROOT_PROP, "")
    if session:
        return session
    scene_root = getattr(context.scene, _GAME_ROOT_PROP, "")
    if scene_root:
        return scene_root
    prefs = _get_addon_preferences(context)
    if prefs is not None and prefs.game_root:
        return prefs.game_root
    return ""


class SCN_OT_exporter(bpy.types.Operator):

    '''Exports a scene to an AeonGames Scene (SCN) file'''
    bl_idname = "export_scene.scn"
    bl_label = "Export AeonGames Scene"

    directory: bpy.props.StringProperty(subtype='DIR_PATH')
    game_root: bpy.props.StringProperty(
        name="Game Root",
        subtype='DIR_PATH',
        description="Root directory assets are exported under. Defaults to the "
                    "session value, then the per-.blend override, then the "
                    "global add-on preference. When set, the output directory "
                    "is <game root>/<resource prefix>",
        default=""
    )
    remember_game_root: bpy.props.BoolProperty(
        name="Save Game Root in .blend",
        description="Also store the game root on the scene so it is saved "
                    "inside this .blend file as a per-project override",
        default=False
    )
    as_text: bpy.props.BoolProperty(
        name="Export as Text",
        description="Write the scene and every generated model/mesh/material as human-readable .txt files instead of binary",
        default=False
    )
    export_models: bpy.props.BoolProperty(
        name="Models",
        description="Export one model (.mdl) per unique mesh datablock via io_model_mdl; objects sharing a datablock become instance nodes referencing the same model",
        default=True
    )
    resource_prefix: bpy.props.StringProperty(
        name="Resource Prefix",
        description="Path prefix (relative to the game resource root) prepended to generated model references, e.g. 'sponza'",
        default=""
    )
    # Mesh vertex attribute toggles (forwarded to io_model_mdl -> io_mesh_msh).
    export_tangents: bpy.props.BoolProperty(name="Tangents", default=True)
    export_uvs: bpy.props.BoolProperty(name="UVs", default=True)
    export_weights: bpy.props.BoolProperty(name="Weights", default=True)
    export_colors: bpy.props.BoolProperty(name="Vertex Colors", default=True)

    @classmethod
    def poll(cls, context):
        is_mdl_enabled, is_mdl_loaded = addon_utils.check('io_model_mdl')
        return is_mdl_loaded and is_mdl_enabled

    @staticmethod
    def safe_name(name):
        # Make a Blender datablock name safe to use as a filename component.
        return "".join(c if (c.isalnum() or c in "._-") else "_" for c in name)

    def draw(self, context):
        layout = self.layout
        root_box = layout.box()
        root_box.label(text="Export Location")
        root_box.prop(self, "game_root")
        root_box.prop(self, "resource_prefix")
        root_box.prop(self, "remember_game_root")
        layout.separator()
        layout.prop(self, "export_models")
        layout.separator()
        attr_col = layout.column(heading="Mesh Attributes")
        attr_col.enabled = self.export_models
        attr_col.prop(self, "export_tangents")
        attr_col.prop(self, "export_uvs")
        attr_col.prop(self, "export_weights")
        attr_col.prop(self, "export_colors")
        layout.separator()
        layout.prop(self, "as_text")

    # -- transform helpers --------------------------------------------------
    @staticmethod
    def _set_transform(transform_msg, matrix):
        translation, rotation, scale = matrix.decompose()
        transform_msg.Scale.x = scale.x
        transform_msg.Scale.y = scale.y
        transform_msg.Scale.z = scale.z
        # mathutils.Quaternion is (w, x, y, z); QuaternionMsg is (w, x, y, z).
        transform_msg.Rotation.w = rotation.w
        transform_msg.Rotation.x = rotation.x
        transform_msg.Rotation.y = rotation.y
        transform_msg.Rotation.z = rotation.z
        transform_msg.Translation.x = translation.x
        transform_msg.Translation.y = translation.y
        transform_msg.Translation.z = translation.z

    @staticmethod
    def _add_float_property(component_msg, name, value):
        prop = component_msg.property.add()
        prop.name = name
        setattr(prop, "float", float(value))

    # -- component builders -------------------------------------------------
    def _add_model_component(self, node_msg, model_path):
        component = node_msg.component.add()
        component.name = "Model Component"
        prop = component.property.add()
        prop.name = "Model"
        prop.path = model_path

    def _add_light_component(self, node_msg, light):
        color = light.color  # linear RGB, 0..1
        if light.type == 'SUN':
            component = node_msg.component.add()
            component.name = "Directional Light"
            self._add_float_property(component, "Color R", color[0])
            self._add_float_property(component, "Color G", color[1])
            self._add_float_property(component, "Color B", color[2])
            self._add_float_property(component, "Intensity", light.energy)
        elif light.type == 'SPOT':
            component = node_msg.component.add()
            component.name = "Spot Light"
            self._add_float_property(component, "Color R", color[0])
            self._add_float_property(component, "Color G", color[1])
            self._add_float_property(component, "Color B", color[2])
            self._add_float_property(component, "Intensity", light.energy)
            radius = light.cutoff_distance if light.use_custom_distance else 0.0
            self._add_float_property(component, "Radius", radius)
            # Blender spot_size is the full cone angle; the engine cone uses
            # half-angles. spot_blend (0..1) is the fraction of the cone that
            # is the soft falloff, measured inward from the outer edge.
            outer = light.spot_size * 0.5
            inner = outer * (1.0 - light.spot_blend)
            self._add_float_property(component, "Inner Angle", inner)
            self._add_float_property(component, "Outer Angle", outer)
        else:
            # POINT (and AREA, which has no engine equivalent) -> Point Light.
            component = node_msg.component.add()
            component.name = "Point Light"
            self._add_float_property(component, "Color R", color[0])
            self._add_float_property(component, "Color G", color[1])
            self._add_float_property(component, "Color B", color[2])
            self._add_float_property(component, "Intensity", light.energy)
            radius = light.cutoff_distance if light.use_custom_distance else 0.0
            self._add_float_property(component, "Radius", radius)

    def _add_camera_component(self, node_msg, camera):
        component = node_msg.component.add()
        component.name = "Camera"
        # The engine's Matrix4x4::Perspective takes the VERTICAL field of view
        # in DEGREES (tan(fov * PI / 360)).
        self._add_float_property(component, "Field of View",
                                 math.degrees(camera.angle_y))
        self._add_float_property(component, "Near Plane", camera.clip_start)
        self._add_float_property(component, "Far Plane", camera.clip_end)

    # -- per-mesh model export ----------------------------------------------
    def _export_model(self, context, representative, model_name, outdir):
        '''Export a single mesh datablock as a model, in object-local space,
        by isolating a transform-cleared copy of one representative object in
        the active scene and driving the existing io_model_mdl exporter with
        its "Selected Only" mode. The copy is named after the model so the
        generated mesh/material files get clean, stable filenames.'''
        scene = context.scene
        view_layer = context.view_layer
        copies = []
        renamed = []  # (object, original_name) pairs to restore
        prev_selection = context.selected_objects[:]
        prev_active = view_layer.objects.active
        try:
            # Free the model name so the copy can claim it verbatim (object
            # names are unique; an existing holder would otherwise force ".001"
            # onto the copy and hence onto every generated mesh file).
            existing = bpy.data.objects.get(model_name)
            if existing is not None:
                renamed.append((existing, existing.name))
                existing.name = "__scn_tmp__" + model_name

            obj_copy = representative.copy()  # shares the mesh datablock
            obj_copy.name = model_name
            obj_copy.matrix_world = mathutils.Matrix.Identity(4)
            scene.collection.objects.link(obj_copy)
            copies.append(obj_copy)
            # Bring along the rig so io_model_mdl can emit skeleton/animations.
            for modifier in representative.modifiers:
                if modifier.type == 'ARMATURE' and modifier.object is not None:
                    armature_copy = modifier.object.copy()
                    armature_copy.matrix_world = mathutils.Matrix.Identity(4)
                    scene.collection.objects.link(armature_copy)
                    copies.append(armature_copy)
                    # Re-point the copied modifier at the copied armature.
                    for mod in obj_copy.modifiers:
                        if mod.type == 'ARMATURE' and mod.object == modifier.object:
                            mod.object = armature_copy

            # Realize the new object bases, then isolate the copies in the
            # selection so io_model_mdl (Selected Only) exports just this mesh.
            view_layer.update()
            for obj in scene.objects:
                obj.select_set(False)
            for obj_copy in copies:
                obj_copy.select_set(True)
            view_layer.objects.active = copies[0]

            bpy.ops.export_model.mdl(
                'EXEC_DEFAULT',
                directory=outdir + os.sep,
                as_text=self.as_text,
                export_meshes=True,
                export_skeleton=True,
                export_animations=True,
                export_textures=True,
                export_materials=True,
                resource_prefix=self.resource_prefix,
                selected_only=True,
                export_tangents=self.export_tangents,
                export_uvs=self.export_uvs,
                export_weights=self.export_weights,
                export_colors=self.export_colors)
        finally:
            for obj_copy in copies:
                data = obj_copy.data
                bpy.data.objects.remove(obj_copy)
                # Never remove the shared mesh datablock; only drop an
                # orphaned armature datablock created for the rig copy.
                if (data is not None and data.users == 0
                        and isinstance(data, bpy.types.Armature)):
                    bpy.data.armatures.remove(data)
            for obj, original_name in renamed:
                obj.name = original_name
            # Restore the user's original selection/active object.
            view_layer.update()
            for obj in scene.objects:
                obj.select_set(obj in prev_selection)
            if prev_active is not None:
                view_layer.objects.active = prev_active

    def execute(self, context):
        prefix = (self.resource_prefix + "/") if self.resource_prefix else ""
        model_ext = ".txt" if self.as_text else ".mdl"

        # Resolve and persist the game root. When set, it is the export root
        # and the output directory is <game root>/<resource prefix>; otherwise
        # fall back to the directory chosen in the file browser.
        game_root = self.game_root.strip()
        if game_root:
            # Session-sticky: remember for further exports this session and let
            # lower-layer exporters default to the same root.
            context.window_manager.aeon_game_root = game_root
            if self.remember_game_root:
                context.scene.aeon_game_root = game_root
            rel = self.resource_prefix.strip().strip("/\\")
            outdir = os.path.join(game_root, *rel.split("/")) if rel else game_root
            os.makedirs(outdir, exist_ok=True)
        else:
            outdir = self.directory.rstrip("/\\")

        scene = context.scene
        scene_buffer = scene_pb2.SceneMsg()
        scene_buffer.name = scene.name

        # 1. One model per unique mesh datablock. Remember the model path so
        #    every object using that datablock becomes an instance node.
        mesh_to_model = {}
        if self.export_models:
            original_scene_name = scene.name
            for obj in scene.objects:
                if obj.type != 'MESH' or obj.data in mesh_to_model:
                    continue
                model_name = self.safe_name(obj.data.name)
                print("Exporting model", model_name, "from mesh datablock",
                      obj.data.name)
                # io_model_mdl derives the .mdl filename from the scene name.
                scene.name = model_name
                try:
                    self._export_model(context, obj, model_name, outdir)
                finally:
                    scene.name = original_scene_name
                mesh_to_model[obj.data] = prefix + model_name + model_ext
            scene_buffer.name = original_scene_name

        # 2. One node per object (mesh instances, lights, cameras).
        camera_nodes = {}
        for obj in scene.objects:
            if obj.type == 'MESH':
                model_path = mesh_to_model.get(obj.data)
                if model_path is None:
                    print("Skipping mesh node", obj.name, "(no model exported)")
                    continue
                node = scene_buffer.node.add()
                node.name = obj.name
                self._set_transform(node.local, obj.matrix_world)
                self._add_model_component(node, model_path)
            elif obj.type == 'LIGHT':
                node = scene_buffer.node.add()
                node.name = obj.name
                self._set_transform(node.local, obj.matrix_world)
                self._add_light_component(node, obj.data)
            elif obj.type == 'CAMERA':
                node = scene_buffer.node.add()
                node.name = obj.name
                self._set_transform(node.local, obj.matrix_world)
                self._add_camera_component(node, obj.data)
                camera_nodes[obj] = obj.data
            else:
                print("Skipping object", obj.name, "of type", obj.type)

        # 3. Scene-level active camera (drives the default view/projection).
        active_camera = scene.camera
        if active_camera is None and camera_nodes:
            active_camera = next(iter(camera_nodes))
        if active_camera is not None and active_camera.type == 'CAMERA':
            camera = active_camera.data
            scene_buffer.camera.node = active_camera.name
            scene_buffer.camera.field_of_view = math.degrees(camera.angle_y)
            scene_buffer.camera.near_plane = camera.clip_start
            scene_buffer.camera.far_plane = camera.clip_end

        # 4. Write the scene file. In text mode the model files also use a
        #    .txt extension, so disambiguate the scene basename if it would
        #    collide (case-insensitively, for Windows/macOS) with a model file.
        scene_base = self.safe_name(scene.name)
        if self.as_text:
            model_bases = {os.path.splitext(os.path.basename(path))[0].lower()
                           for path in mesh_to_model.values()}
            if scene_base.lower() in model_bases:
                scene_base += "_scene"
            scn_path = os.path.join(outdir, scene_base + ".txt")
            print("Writing", scn_path, ".")
            with open(scn_path, "wt") as out:
                out.write("AEONSCN\n")
                out.write(google.protobuf.text_format.MessageToString(
                    scene_buffer))
        else:
            scn_path = os.path.join(outdir, scene_base + ".scn")
            print("Writing", scn_path, ".")
            with open(scn_path, "wb") as out:
                out.write(struct.Struct('8s').pack(b'AEONSCN\x00'))
                out.write(scene_buffer.SerializeToString())
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        # Seed the game root from the session/scene/preference chain and point
        # the file browser at the resolved <game root>/<resource prefix>.
        if not self.game_root:
            self.game_root = resolve_game_root(context)
        if self.game_root and not self.directory:
            rel = self.resource_prefix.strip().strip("/\\")
            self.directory = (os.path.join(self.game_root, *rel.split("/"))
                              if rel else self.game_root)
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

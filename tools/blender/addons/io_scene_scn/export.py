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

# Blender cameras (and spot/sun lights) look down their local -Z axis with +Y
# up. The engine convention is +X right, +Y forward (the view direction), +Z up
# (see Matrix4x4::Frustum, "glFrustum, +X right, +Y forward, +Z up"). Rotating
# a Blender camera node by -90 degrees about its local X axis maps the engine
# frame onto the Blender frame: engine +Y (forward) -> Blender -Z (view dir),
# engine +Z (up) -> Blender +Y (up), engine +X (right) -> Blender +X (right).
# The node transform is post-multiplied by this so the exported orientation
# makes the engine camera point the same way as in Blender.
_BLENDER_TO_ENGINE_CAMERA = mathutils.Matrix.Rotation(math.radians(-90.0), 4, 'X')

# The engine's forward Phong shaders (diffuse_map_phong*.txt) write radiance
# straight to the framebuffer -- there is no exposure, tone mapping or gamma,
# so any accumulated radiance above ~1.0 clips to white. Blender's light.energy
# values are physically based (watts for point/spot, W*m^-2 for sun) and tuned
# for its tone-mapped path tracer, so they blow the LDR image out. Scale them
# into a roughly [0..1] radiance range. These are art-calibration constants;
# tune to taste.
_SUN_INTENSITY_SCALE = 0.2     # W*m^-2 -> engine directional radiance
_POINT_INTENSITY_SCALE = 0.01  # W      -> engine point / spot radiance


def _engine_light_radius(light):
    '''Finite range (in world units) for a point/spot light. Blender lights
    without a custom distance have infinite inverse-square range, but the
    engine's falloff is a finite windowed sphere (clamp(1 - d^2/r^2)^2), so a
    radius of 0 makes the light contribute nothing. Derive a usable range from
    the light power when no explicit cutoff is set.'''
    if light.use_custom_distance:
        return light.cutoff_distance
    return math.sqrt(max(light.energy, 0.0))


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


def is_valid_game_root(game_root):
    '''A game root is valid when it resolves to an existing directory.'''
    if not game_root or not game_root.strip():
        return False
    return os.path.isdir(bpy.path.abspath(game_root.strip()))


class SCN_OT_exporter(bpy.types.Operator):

    '''Exports a scene to an AeonGames Scene (SCN) file'''
    bl_idname = "export_scene.scn"
    bl_label = "Export AeonGames Scene"

    game_root: bpy.props.StringProperty(
        name="Game Root",
        subtype='DIR_PATH',
        description="Root directory all assets are exported under. The scene "
                    "file is written to <game root>/scenes/<scene name>.scn and "
                    "every other asset under <game root>/<scene name>/. A valid "
                    "(existing) game root is required",
        default=""
    )
    remember_game_root: bpy.props.BoolProperty(
        name="Save Game Root in .blend",
        description="Also store the game root on the scene so it is saved "
                    "inside this .blend file as a per-project override",
        default=True
    )
    export_name: bpy.props.StringProperty(
        name="Scene Name",
        description="Overrides the name used for the output folder and scene "
                    "file. Assets go under <game root>/<scene name>/ and the "
                    "scene file under <game root>/scenes/<scene name>.scn. "
                    "Leave empty to use the Blender scene name. Saved in this "
                    ".blend file",
        default=""
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
    force: bpy.props.BoolProperty(
        name="Force Overwrite",
        description="Re-export every model even if its file already exists. "
                    "When off, existing model files are reused and only the "
                    "scene file is rewritten",
        default=False
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

    @staticmethod
    def _is_hidden(obj):
        '''An object counts as hidden (and is skipped on export) when it is
        hidden in the viewport (eye icon), globally disabled in viewports
        (monitor icon) or excluded from renders (camera icon).'''
        if obj.hide_viewport or obj.hide_render:
            return True
        try:
            return obj.hide_get()
        except RuntimeError:
            # hide_get() needs a view-layer context; treat as visible if it
            # cannot be evaluated.
            return False

    def draw(self, context):
        layout = self.layout
        root_box = layout.box()
        root_box.label(text="Export Location")
        root_box.prop(self, "game_root")
        if not is_valid_game_root(self.game_root):
            root_box.label(text="Game Root must be an existing directory",
                           icon='ERROR')
        root_box.prop(self, "export_name",
                      text="Scene Name (%s)" % context.scene.name)
        root_box.prop(self, "remember_game_root")
        layout.separator()
        layout.prop(self, "export_models")
        layout.prop(self, "force")
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
            self._add_float_property(component, "Intensity",
                                     light.energy * _SUN_INTENSITY_SCALE)
        elif light.type == 'SPOT':
            component = node_msg.component.add()
            component.name = "Spot Light"
            self._add_float_property(component, "Color R", color[0])
            self._add_float_property(component, "Color G", color[1])
            self._add_float_property(component, "Color B", color[2])
            self._add_float_property(component, "Intensity",
                                     light.energy * _POINT_INTENSITY_SCALE)
            self._add_float_property(component, "Radius",
                                     _engine_light_radius(light))
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
            self._add_float_property(component, "Intensity",
                                     light.energy * _POINT_INTENSITY_SCALE)
            self._add_float_property(component, "Radius",
                                     _engine_light_radius(light))

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
    def _export_model(self, context, representative, model_name, outdir,
                      resource_prefix):
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
                resource_prefix=resource_prefix,
                selected_only=True,
                force=self.force,
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
        model_ext = ".txt" if self.as_text else ".mdl"
        scene = context.scene

        # A valid (existing) game root is mandatory.
        game_root = self.game_root.strip()
        if not is_valid_game_root(game_root):
            self.report({'ERROR'}, "Game Root must be set to an existing "
                        "directory before exporting.")
            return {'CANCELLED'}
        abs_root = bpy.path.abspath(game_root)

        # Resolve the export name (output folder + scene file basename).
        export_name = self.export_name.strip() or scene.name
        name = self.safe_name(export_name)

        # Persist choices: session-sticky game root + per-.blend overrides.
        context.window_manager.aeon_game_root = game_root
        if self.remember_game_root:
            scene.aeon_game_root = game_root
        scene.aeon_export_name = self.export_name.strip()

        # Layout: scene file under <root>/scenes, all other assets under
        # <root>/<name>. References inside the scene are game-root-relative,
        # so models are referenced as "<name>/<model>".
        assets_dir = os.path.join(abs_root, name)
        scenes_dir = os.path.join(abs_root, "scenes")
        os.makedirs(assets_dir, exist_ok=True)
        os.makedirs(scenes_dir, exist_ok=True)
        prefix = name + "/"

        scene_buffer = scene_pb2.SceneMsg()
        scene_buffer.name = export_name

        # 1. One model per unique mesh datablock. Remember the model path so
        #    every object using that datablock becomes an instance node.
        mesh_to_model = {}
        if self.export_models:
            # Pre-collect the unique mesh datablocks so we can report progress.
            representatives = []
            seen = set()
            for obj in scene.objects:
                if obj.type == 'MESH' and not self._is_hidden(obj) \
                        and obj.data not in seen:
                    seen.add(obj.data)
                    representatives.append(obj)
            total = len(representatives)

            wm = context.window_manager
            wm.progress_begin(0, total)
            original_scene_name = scene.name
            try:
                for index, obj in enumerate(representatives):
                    model_name = self.safe_name(obj.data.name)
                    model_path = os.path.join(assets_dir, model_name + model_ext)
                    mesh_to_model[obj.data] = prefix + model_name + model_ext
                    # Reuse an existing model unless forced; the scene file is
                    # always rewritten regardless.
                    if not self.force and os.path.exists(model_path):
                        print("[%d/%d] Skipping model %s (already exists)"
                              % (index + 1, total, model_name))
                        wm.progress_update(index + 1)
                        continue
                    print("[%d/%d] Exporting model %s from mesh datablock %s"
                          % (index + 1, total, model_name, obj.data.name))
                    self.report({'INFO'}, "Exporting model %d/%d: %s"
                                % (index + 1, total, model_name))
                    # io_model_mdl derives the .mdl filename from scene.name.
                    scene.name = model_name
                    try:
                        self._export_model(context, obj, model_name,
                                           assets_dir, name)
                    finally:
                        scene.name = original_scene_name
                    wm.progress_update(index + 1)
            finally:
                wm.progress_end()
            scene_buffer.name = export_name

        # 2. One node per object (mesh instances, lights, cameras). Hidden
        #    objects are skipped so they do not appear in the exported scene.
        camera_nodes = {}
        for obj in scene.objects:
            if self._is_hidden(obj):
                print("Skipping hidden object", obj.name)
                continue
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
                # Reorient from Blender's -Z-forward/+Y-up camera frame to the
                # engine's +Y-forward/+Z-up frame so the view direction matches.
                self._set_transform(node.local,
                                    obj.matrix_world @ _BLENDER_TO_ENGINE_CAMERA)
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

        # 3b. Clustered Forward+ lighting pipeline. The engine only runs the
        #     light-culling/compute passes (and therefore shades anything with
        #     scene lights) when the scene references a lighting pipeline;
        #     without it geometry renders unlit. Default to the engine's
        #     stock lighting program so exported scenes are lit out of the box.
        scene_buffer.lighting_pipeline.path = "shaders/lighting.txt"

        # 4. Write the scene file under <game root>/scenes. It lives in its own
        #    directory, separate from the per-scene asset folder, so there is no
        #    filename collision with the generated models.
        if self.as_text:
            scn_path = os.path.join(scenes_dir, name + ".txt")
            print("Writing", scn_path, ".")
            with open(scn_path, "wt") as out:
                out.write("AEONSCN\n")
                out.write(google.protobuf.text_format.MessageToString(
                    scene_buffer))
        else:
            scn_path = os.path.join(scenes_dir, name + ".scn")
            print("Writing", scn_path, ".")
            with open(scn_path, "wb") as out:
                out.write(struct.Struct('8s').pack(b'AEONSCN\x00'))
                out.write(scene_buffer.SerializeToString())
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        # Seed the game root from the session/scene/preference chain and the
        # scene-name override from the per-.blend property, then show a plain
        # properties dialog (no file browser).
        if not self.game_root:
            self.game_root = resolve_game_root(context)
        if not self.export_name:
            self.export_name = getattr(context.scene, "aeon_export_name", "")
        return context.window_manager.invoke_props_dialog(self, width=400)

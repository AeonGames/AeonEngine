# Copyright (C) 2017-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba
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
import addon_utils
import model_pb2
import google.protobuf.text_format

class MDL_OT_exporter(bpy.types.Operator):

    '''Exports a scene to an AeonGames Model (MDL) file'''
    bl_idname = "export_model.mdl"
    bl_label = "Export AeonGames Model"

    directory: bpy.props.StringProperty(subtype='DIR_PATH')
    as_text: bpy.props.BoolProperty(
        name="Export as Text",
        description="Write all protobuf messages (model, meshes, skeletons, animations) as human-readable .txt files instead of binary",
        default=False
    )
    export_meshes: bpy.props.BoolProperty(
        name="Meshes",
        description="Export each MESH object to a .msh file under meshes/ and add it to the model",
        default=True
    )
    export_skeleton: bpy.props.BoolProperty(
        name="Skeleton",
        description="Export the first ARMATURE object to a .skl file under skeletons/ and reference it from the model",
        default=True
    )
    export_animations: bpy.props.BoolProperty(
        name="Animations",
        description="Export every action in the .blend to .anm files under animations/ and reference them from the model",
        default=True
    )
    export_textures: bpy.props.BoolProperty(
        name="Textures",
        description="Save image-texture nodes referenced by exported meshes into textures/",
        default=True
    )
    export_materials: bpy.props.BoolProperty(
        name="Materials",
        description="Export a Phong material (.mtl) per used material slot under materials/ and reference it from each assembly",
        default=True
    )
    resource_prefix: bpy.props.StringProperty(
        name="Resource Prefix",
        description="Path prefix (relative to the game resource root) prepended to generated mesh/material/texture references, e.g. 'sponza'",
        default=""
    )
    selected_only: bpy.props.BoolProperty(
        name="Selected Only",
        description="Export only the currently selected objects instead of every object in the scene",
        default=False
    )
    force: bpy.props.BoolProperty(
        name="Force Overwrite",
        description="Re-export every mesh, material, skeleton, animation and "
                    "texture even if its file already exists. The model (.mdl) "
                    "file is always rewritten",
        default=False
    )
    # Mesh vertex attribute toggles (forwarded to io_mesh_msh).
    export_tangents: bpy.props.BoolProperty(
        name="Tangents",
        description="Export tangent and bitangent vectors (requires UVs)",
        default=True
    )
    export_uvs: bpy.props.BoolProperty(
        name="UVs",
        description="Export texture coordinates",
        default=True
    )
    export_weights: bpy.props.BoolProperty(
        name="Weights",
        description="Export vertex weights for skeletal animation",
        default=True
    )
    export_colors: bpy.props.BoolProperty(
        name="Vertex Colors",
        description="Export vertex color attributes",
        default=True
    )

    @classmethod
    def poll(cls, context):
        is_msh_enabled, is_msh_loaded = addon_utils.check('io_mesh_msh')
        is_skl_enabled, is_skl_loaded = addon_utils.check('io_skeleton_skl')
        return is_msh_loaded and is_msh_enabled and is_skl_loaded and is_skl_enabled

    @staticmethod
    def safe_name(name):
        # Make a Blender datablock name safe to use as a filename component.
        return "".join(c if (c.isalnum() or c in "._-") else "_" for c in name)

    def draw(self, context):
        layout = self.layout
        col = layout.column(heading="Include")
        col.prop(self, "export_meshes")
        col.prop(self, "export_skeleton")
        col.prop(self, "export_animations")
        col.prop(self, "export_textures")
        col.prop(self, "export_materials")
        layout.separator()
        layout.prop(self, "resource_prefix")
        layout.prop(self, "force")
        layout.separator()
        attr_col = layout.column(heading="Mesh Attributes")
        attr_col.enabled = self.export_meshes
        attr_col.prop(self, "export_tangents")
        attr_col.prop(self, "export_uvs")
        attr_col.prop(self, "export_weights")
        attr_col.prop(self, "export_colors")
        layout.separator()
        layout.prop(self, "as_text")

    def execute(self, context):
        is_anm_enabled, is_anm_loaded = addon_utils.check('io_animation_anm')
        animations_available = is_anm_enabled and is_anm_loaded
        if self.export_animations and not animations_available:
            print("Animations requested but io_animation_anm addon is not enabled; skipping animation export.")

        is_mtl_enabled, is_mtl_loaded = addon_utils.check('io_material_mtl')
        materials_available = is_mtl_enabled and is_mtl_loaded
        if self.export_materials and not materials_available:
            print("Materials requested but io_material_mtl addon is not enabled; skipping material export.")

        # Path prefix (relative to the resource root) for generated references.
        resource_prefix = (self.resource_prefix + "/") if self.resource_prefix else ""

        if self.export_meshes and not os.path.exists(self.directory + "meshes"):
            os.makedirs(self.directory + "meshes")
        if self.export_skeleton and not os.path.exists(self.directory + "skeletons"):
            os.makedirs(self.directory + "skeletons")
        if self.export_textures and not os.path.exists(self.directory + "textures"):
            os.makedirs(self.directory + "textures")
        if self.export_materials and materials_available and not os.path.exists(self.directory + "materials"):
            os.makedirs(self.directory + "materials")
        if self.export_animations and animations_available and not os.path.exists(self.directory + "animations"):
            os.makedirs(self.directory + "animations")

        model_buffer = model_pb2.ModelMsg()
        # Store original selection
        original_selection = context.selected_objects[:]
        original_active = context.view_layer.objects.active

        objects_source = (context.selected_objects[:] if self.selected_only
                          else context.scene.objects)
        for object in objects_source:
            # Make this object the only selected and active object
            bpy.ops.object.select_all(action='DESELECT')
            object.select_set(True)
            context.view_layer.objects.active = object
            
            if object.type == 'MESH':
                if not self.export_meshes:
                    print("Skipping mesh", object.name, "(meshes disabled)")
                    continue
                print("Exporting", object.name, "of type", object.type)
                mesh_ext = ".txt" if self.as_text else ".msh"
                mat_ext = ".txt" if self.as_text else ".mtl"
                # Skinning is applied in a compute pre-pass, so meshes are drawn
                # with the static-mesh clustered Phong pipeline regardless of rig state.
                pipeline_path = "shaders/clustered_phong.txt"

                materials = object.data.materials
                # Determine which material slots actually have geometry.
                used_indices = sorted({polygon.material_index
                                       for polygon in object.data.polygons})
                used_indices = [index for index in used_indices
                                if index < len(materials) and materials[index] is not None]
                # Only suffix mesh names when a single object spans several slots.
                split = len(used_indices) > 1

                if used_indices:
                    for material_index in used_indices:
                        material = materials[material_index]
                        mesh_name = (object.name + "_" + self.safe_name(material.name)
                                     if split else object.name)
                        assembly = model_buffer.assembly.add()
                        assembly.mesh.path = resource_prefix + "meshes/" + mesh_name + mesh_ext
                        assembly.pipeline.path = pipeline_path
                        mesh_file = self.directory + "meshes" + os.sep + mesh_name + mesh_ext
                        if self.force or not os.path.exists(mesh_file):
                            bpy.ops.export_mesh.msh(
                                'EXEC_DEFAULT',
                                filepath=self.directory + "meshes" + os.sep + mesh_name + ".msh",
                                as_text=self.as_text,
                                export_tangents=self.export_tangents,
                                export_uvs=self.export_uvs,
                                export_weights=self.export_weights,
                                export_colors=self.export_colors,
                                material_index=material_index)
                        else:
                            print("\tSkipping mesh (already exists):", mesh_file)
                        if self.export_materials and materials_available:
                            material_name = self.safe_name(material.name)
                            assembly.material.path = resource_prefix + "materials/" + material_name + mat_ext
                            material_file = self.directory + "materials" + os.sep + material_name + mat_ext
                            if self.force or not os.path.exists(material_file):
                                bpy.ops.export_material.mtl(
                                    'EXEC_DEFAULT',
                                    filepath=self.directory + "materials" + os.sep + material_name + ".mtl",
                                    material_name=material.name,
                                    texture_dir=resource_prefix + "textures",
                                    as_text=self.as_text)
                            else:
                                print("\tSkipping material (already exists):", material_file)
                else:
                    # No usable material slots: export the whole mesh as one assembly.
                    assembly = model_buffer.assembly.add()
                    assembly.mesh.path = resource_prefix + "meshes/" + object.name + mesh_ext
                    assembly.pipeline.path = pipeline_path
                    mesh_file = self.directory + "meshes" + os.sep + object.name + mesh_ext
                    if self.force or not os.path.exists(mesh_file):
                        bpy.ops.export_mesh.msh(
                            'EXEC_DEFAULT',
                            filepath=self.directory + "meshes" + os.sep + object.name + ".msh",
                            as_text=self.as_text,
                            export_tangents=self.export_tangents,
                            export_uvs=self.export_uvs,
                            export_weights=self.export_weights,
                            export_colors=self.export_colors)
                    else:
                        print("\tSkipping mesh (already exists):", mesh_file)
                if not self.export_textures:
                    continue
                # Export all textures referenced by the mesh materials
                for material in object.data.materials:
                    if material is None or material.node_tree is None:
                        continue
                    print("Material:",material.name, material.use_nodes)
                    for node in material.node_tree.nodes:
                        print("\tNode:",node.bl_idname,node.label)
                        if node.bl_idname == "ShaderNodeTexImage" and node.image is not None:
                            original_filepath = node.image.filepath_raw
                            # Pick a basename: prefer the existing filepath,
                            # but fall back to the image's datablock name
                            # (e.g. for packed or generated images that have
                            # no on-disk source).
                            basename = os.path.basename(original_filepath) if original_filepath else ""
                            if not basename:
                                basename = node.image.name
                                # Ensure we have a file extension so OIIO
                                # can pick a writer.
                                if not os.path.splitext(basename)[1]:
                                    ext_map = {
                                        'PNG':  '.png',
                                        'JPEG': '.jpg',
                                        'JPEG2000': '.jp2',
                                        'TARGA': '.tga',
                                        'TARGA_RAW': '.tga',
                                        'BMP':  '.bmp',
                                        'TIFF': '.tif',
                                        'OPEN_EXR': '.exr',
                                        'OPEN_EXR_MULTILAYER': '.exr',
                                        'HDR':  '.hdr',
                                        'WEBP': '.webp',
                                    }
                                    basename += ext_map.get(node.image.file_format, '.png')
                            target = os.path.join(self.directory, "textures", basename)
                            # Skip if the file already exists at the
                            # destination, and especially if the source
                            # image on disk *is* the destination -- saving
                            # in-place can corrupt the image (Blender
                            # truncates before reading) and at best is just
                            # wasted I/O.
                            try:
                                source_abs = os.path.normcase(os.path.abspath(
                                    bpy.path.abspath(original_filepath))) if original_filepath else ""
                            except Exception:
                                source_abs = ""
                            target_abs = os.path.normcase(os.path.abspath(target))
                            if os.path.exists(target):
                                if source_abs == target_abs:
                                    print("\t\tSkipping image (source is destination):", target)
                                    continue
                                if not self.force:
                                    print("\t\tSkipping image (already exists):", target)
                                    continue
                            node.image.filepath_raw = target
                            node.image.save()
                            node.image.filepath_raw = original_filepath
            elif object.type == 'ARMATURE':
                # Export armature as skeleton file
                if self.export_skeleton:
                    print("Exporting", object.name, "of type", object.type)
                    skl_ext = ".txt" if self.as_text else ".skl"
                    skeleton_filepath = self.directory + "skeletons" + os.sep + object.name + ".skl"
                    skeleton_file = self.directory + "skeletons" + os.sep + object.name + skl_ext
                    if self.force or not os.path.exists(skeleton_file):
                        bpy.ops.export_skeleton.skl(
                                            'EXEC_DEFAULT',
                                            filepath=skeleton_filepath,
                                            as_text=self.as_text)
                    else:
                        print("\tSkipping skeleton (already exists):", skeleton_file)
                    # Reference skeleton in model if this is the first armature
                    if not model_buffer.HasField('skeleton'):
                        model_buffer.skeleton.path = resource_prefix + "skeletons/" + object.name + skl_ext
                else:
                    print("Skipping skeleton", object.name, "(skeleton disabled)")
                # Export all actions for this armature as .anm files.
                if self.export_animations and animations_available and object.animation_data is not None:
                    anm_ext = ".txt" if self.as_text else ".anm"
                    anm_dir = self.directory + "animations" + os.sep
                    # The anm exporter writes every action at once; skip the
                    # whole call only when not forcing and all already exist.
                    missing = any(
                        not os.path.exists(anm_dir + action.name + anm_ext)
                        for action in bpy.data.actions)
                    if self.force or missing:
                        print("Exporting animations for", object.name)
                        bpy.ops.export_armature.anm(
                            'EXEC_DEFAULT',
                            directory=anm_dir,
                            as_text=self.as_text)
                    else:
                        print("\tSkipping animations (already exist) for", object.name)
                    for action in bpy.data.actions:
                        ref = model_buffer.animation.add()
                        ref.name = action.name
                        ref.reference.path = resource_prefix + "animations/" + action.name + anm_ext
            else:
                print("Skipping object", object.name, "of type", object.type)
        
        # Restore original selection
        bpy.ops.object.select_all(action='DESELECT')
        for obj in original_selection:
            obj.select_set(True)
        context.view_layer.objects.active = original_active
        mdl_filepath = self.directory + os.sep + context.scene.name + ".mdl"
        if self.as_text:
            text_path = mdl_filepath.replace('.mdl', '.txt')
            print("Writting", text_path, ".")
            out = open(text_path, "wt")
            out.write("AEONMDL\n")
            out.write(
                google.protobuf.text_format.MessageToString(model_buffer))
            out.close()
            print("Done.")
        else:
            print("Writting", mdl_filepath, ".")
            out = open(mdl_filepath, "wb")
            magick_struct = struct.Struct('8s')
            out.write(magick_struct.pack(b'AEONMDL\x00'))
            out.write(model_buffer.SerializeToString())
            out.close()
            print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

# Copyright (C) 2017-2019,2021,2025 Rodrigo Jose Hernandez Cordoba
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

    @classmethod
    def poll(cls, context):
        is_msh_enabled, is_msh_loaded = addon_utils.check('io_mesh_msh')
        is_skl_enabled, is_skl_loaded = addon_utils.check('io_skeleton_skl')
        return is_msh_loaded and is_msh_enabled and is_skl_loaded and is_skl_enabled

    def execute(self, context):
        if not os.path.exists(self.directory + "meshes"):
            os.makedirs(self.directory + "meshes")
        if not os.path.exists(self.directory + "skeletons"):
            os.makedirs(self.directory + "skeletons")
        if not os.path.exists(self.directory + "images"):
            os.makedirs(self.directory + "images")

        model_buffer = model_pb2.ModelMsg()
        # Store original selection
        original_selection = context.selected_objects[:]
        original_active = context.view_layer.objects.active
        
        for object in context.scene.objects:
            # Make this object the only selected and active object
            bpy.ops.object.select_all(action='DESELECT')
            object.select_set(True)
            context.view_layer.objects.active = object
            
            if object.type == 'MESH':
                print("Exporting", object.name, "of type", object.type)
                assembly = model_buffer.assembly.add()
                assembly.mesh.path = "meshes" + '/' + object.name + ".msh"
                bpy.ops.export_mesh.msh(
                    'EXEC_DEFAULT',
                    filepath=self.directory + assembly.mesh.path)
                # Export All images referenced by the mesh materials
                for material in object.data.materials:
                    print("Material:",material.name, material.use_nodes)
                    for node in material.node_tree.nodes:
                        print("\tNode:",node.bl_idname,node.label)
                        if node.bl_idname == "ShaderNodeTexImage":
                            filepath = node.image.filepath_raw
                            node.image.filepath_raw = self.directory + '/' + "images" + '/' + os.path.basename(filepath)
                            node.image.save()
                            node.image.filepath_raw = filepath
            elif object.type == 'ARMATURE':
                # Export armature as skeleton file
                print("Exporting", object.name, "of type", object.type)
                skeleton_filepath = self.directory + "skeletons" + '/' + object.name + ".skl"
                bpy.ops.export_skeleton.skl(
                                    'EXEC_DEFAULT',
                                    filepath=skeleton_filepath)
                # Reference skeleton in model if this is the first armature
                if not model_buffer.HasField('skeleton'):
                    model_buffer.skeleton.path = "skeletons" + '/' + object.name + ".skl"
            else:
                print("Skipping object", object.name, "of type", object.type)
        
        # Restore original selection
        bpy.ops.object.select_all(action='DESELECT')
        for obj in original_selection:
            obj.select_set(True)
        context.view_layer.objects.active = original_active
        print(
            "Writting",
            self.directory +
            os.sep +
            context.scene.name +
            ".mdl",
            ".")
        out = open(self.directory + os.sep + context.scene.name + ".mdl", "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONMDL\x00'))
        out.write(model_buffer.SerializeToString())
        out.close()
        print("Done.")
        mdl_filepath = self.directory + os.sep + context.scene.name + ".mdl"
        print(
            "Writting",
            mdl_filepath.replace('.mdl', '.txt'),
            ".")
        out = open(
            mdl_filepath.replace('.mdl', '.txt'),
            "wt")
        out.write("AEONMDL\n")
        out.write(
            google.protobuf.text_format.MessageToString(model_buffer))
        out.close()
        print("Done.")
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

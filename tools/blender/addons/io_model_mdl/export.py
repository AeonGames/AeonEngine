# Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba
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

    '''Exports an armature to an AeonGames Model (MDL) file'''
    bl_idname = "export_model.mdl"
    bl_label = "Export AeonGames Model"

    directory: bpy.props.StringProperty(subtype='DIR_PATH')

    @classmethod
    def poll(cls, context):
        is_msh_enabled, is_msh_loaded = addon_utils.check('io_mesh_msh')
        return is_msh_loaded

    def execute(self, context):
        if not os.path.exists(self.directory + "meshes"):
            os.makedirs(self.directory + "meshes")
        if not os.path.exists(self.directory + "images"):
            os.makedirs(self.directory + "images")

        model_buffer = model_pb2.ModelMsg()
        for object in context.scene.objects:
            if object.type == 'MESH':
                assembly = model_buffer.assembly.add()
                assembly.mesh.path = "meshes" + os.sep + object.name + ".msh"
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
                            node.image.filepath_raw = self.directory + os.sep + "images" + os.sep + os.path.basename(filepath)
                            node.image.save()
                            node.image.filepath_raw = filepath
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
        print(
            "Writting",
            self.directory +
            os.sep +
            context.scene.name +
            ".mdl.txt",
            ".")
        out = open(
            self.directory +
            os.sep +
            context.scene.name +
            ".mdl.txt",
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

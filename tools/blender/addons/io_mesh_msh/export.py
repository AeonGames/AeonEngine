# Copyright (C) 2016-2019,2021,2025 Rodrigo Jose Hernandez Cordoba
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
import mesh_pb2
import operator
import itertools
import google.protobuf.text_format
from multiprocessing import Pool
from multiprocessing.dummy import Pool as ThreadPool, Lock as ThreadLock


class MSH_OT_exporterCommon():

    def __init__(self, filepath):
        self.mesh = None
        self.object = None
        self.armature = None
        self.vertices = []
        self.indices = []
        self.filepath = filepath

    def get_vertex(self, loop_and_attributes):
        loop = loop_and_attributes[0]
        attributes = loop_and_attributes[1]
        mesh_world_matrix = mathutils.Matrix(self.object.matrix_world)
        vertex = []

        for attribute in attributes:
            if attribute.Semantic == mesh_pb2.AttributeMsg.POSITION:
                vertex.extend(list(self.mesh.vertices[loop.vertex_index].co @ mesh_world_matrix)[:3])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.NORMAL:
                vertex.extend(list(self.mesh.vertices[loop.vertex_index].normal @ mesh_world_matrix)[:3])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.TANGENT:
                vertex.extend(list(self.mesh.vertices[loop.vertex_index].normal @ mesh_world_matrix)[:3])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.BITANGENT:
                vertex.extend(list(self.mesh.vertices[loop.vertex_index].normal @ mesh_world_matrix)[:3])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.TEXCOORD:
                vertex.extend([self.mesh.uv_layers[0].data[loop.index].uv[0],
                            1.0 - self.mesh.uv_layers[0].data[loop.index].uv[1]])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.WEIGHT_INDEX:
                # TODO: Allow for different index size.
                weights = []
                for group in self.mesh.vertices[loop.vertex_index].groups:
                    if group.weight > 0:
                        bone_index = self.armature.bones.find(
                            self.object.vertex_groups[group.group].name)
                        if bone_index < 0 or bone_index > 255:
                            # If the vertex references a bone not in the armature
                            # bone index will be -1
                            print(
                                "Bone index for group",
                                self.object.vertex_groups[
                                    group.group].name,
                                "out of range:",
                                str(bone_index))
                        weights.append([group.weight, bone_index])
                weights.sort()
                weights.reverse()

                # Clip
                if len(weights) > 4:
                    weights = weights[:4]

                # Normalize
                magnitude = 0
                for weight in weights:
                    magnitude = magnitude + weight[0]

                byte_magnitude = 0
                for weight in weights:
                    weight[0] = weight[0] / magnitude
                    weight[0] = int(weight[0] * 0xFF)
                    byte_magnitude = byte_magnitude + weight[0]

                weights[-1][0] = weights[-1][0] + \
                    (0xFF - byte_magnitude)

                if len(weights) < 4:
                    weights = weights + \
                        [[0, 0]] * (4 - len(weights))

                weight_indices = []
                weight_values = []
                weights.sort()
                weights.reverse()
                for weight in weights:
                    weight_indices.append(weight[1])
                    weight_values.append(weight[0])
                vertex.extend(weight_indices)
                vertex.extend(weight_values)
            # Does nothing on mesh_pb2.MeshMsg.WEIGHT_VALUE:
            elif attribute.Semantic == mesh_pb2.AttributeMsg.COLOR:
                # Use color_attributes instead of deprecated vertex_colors
                color_layer = self.mesh.color_attributes[0] if len(self.mesh.color_attributes) > 0 else None
                if color_layer:
                    vertex.extend([color_layer.data[loop.index].color[0],
                                color_layer.data[loop.index].color[1],
                                color_layer.data[loop.index].color[2]])
                else:
                    vertex.extend([1.0, 1.0, 1.0])

        #print("Generating Vertex", loop.index)
        return [loop.index, vertex]

    def get_indices_per_vertex(self, key_group):
        return [list(map(operator.itemgetter(0), key_group[1])), key_group[0]]

    def get_index_dictionary(self, indices, index):
        dictionary = {}
        for key in indices[0]:
            dictionary[key] = index
        return dictionary

    def fill_mesh_buffer(self, mesh_buffer, mesh_object):
        self.mesh = mesh_object.data
        self.object = mesh_object
        pool = ThreadPool()
        # Store center, radii.
        mesh_buffer_min_x = min(
            mesh_object.bound_box[0][0],
            mesh_object.bound_box[1][0],
            mesh_object.bound_box[2][0],
            mesh_object.bound_box[3][0],
            mesh_object.bound_box[4][0],
            mesh_object.bound_box[5][0],
            mesh_object.bound_box[6][0],
            mesh_object.bound_box[7][0])
        mesh_buffer_max_x = max(
            mesh_object.bound_box[0][0],
            mesh_object.bound_box[1][0],
            mesh_object.bound_box[2][0],
            mesh_object.bound_box[3][0],
            mesh_object.bound_box[4][0],
            mesh_object.bound_box[5][0],
            mesh_object.bound_box[6][0],
            mesh_object.bound_box[7][0])
        mesh_buffer_min_y = min(
            mesh_object.bound_box[0][1],
            mesh_object.bound_box[1][1],
            mesh_object.bound_box[2][1],
            mesh_object.bound_box[3][1],
            mesh_object.bound_box[4][1],
            mesh_object.bound_box[5][1],
            mesh_object.bound_box[6][1],
            mesh_object.bound_box[7][1])
        mesh_buffer_max_y = max(
            mesh_object.bound_box[0][1],
            mesh_object.bound_box[1][1],
            mesh_object.bound_box[2][1],
            mesh_object.bound_box[3][1],
            mesh_object.bound_box[4][1],
            mesh_object.bound_box[5][1],
            mesh_object.bound_box[6][1],
            mesh_object.bound_box[7][1])
        mesh_buffer_min_z = min(
            mesh_object.bound_box[0][2],
            mesh_object.bound_box[1][2],
            mesh_object.bound_box[2][2],
            mesh_object.bound_box[3][2],
            mesh_object.bound_box[4][2],
            mesh_object.bound_box[5][2],
            mesh_object.bound_box[6][2],
            mesh_object.bound_box[7][2])
        mesh_buffer_max_z = max(
            mesh_object.bound_box[0][2],
            mesh_object.bound_box[1][2],
            mesh_object.bound_box[2][2],
            mesh_object.bound_box[3][2],
            mesh_object.bound_box[4][2],
            mesh_object.bound_box[5][2],
            mesh_object.bound_box[6][2],
            mesh_object.bound_box[7][2])
        mesh_buffer.Center.x = (
            mesh_buffer_min_x + mesh_buffer_max_x) / 2
        mesh_buffer.Center.y = (
            mesh_buffer_min_y + mesh_buffer_max_y) / 2
        mesh_buffer.Center.z = (
            mesh_buffer_min_z + mesh_buffer_max_z) / 2

        mesh_buffer.Radii.x = mesh_buffer_max_x - mesh_buffer.Center.x
        mesh_buffer.Radii.y = mesh_buffer_max_y - mesh_buffer.Center.y
        mesh_buffer.Radii.z = mesh_buffer_max_z - mesh_buffer.Center.z

        mesh = mesh_object.data

        # if this mesh is modified by an armature, find out which one.
        self.armature = None
        for modifier in mesh_object.modifiers:
            if modifier.type == 'ARMATURE':
                armaturemodifier = bpy.types.ArmatureModifier(modifier)
                if armaturemodifier.use_vertex_groups:
                    if armaturemodifier.object:
                        self.armature = armaturemodifier.object.data
                    break
        vertex_struct_string = ''
        # Position and Normal aren't optional (for now)
        attribute = mesh_buffer.Attribute.add()
        attribute.Semantic   = mesh_pb2.AttributeMsg.POSITION
        attribute.Type       = mesh_pb2.AttributeMsg.FLOAT
        attribute.Size       = 3
        attribute.Flags      = mesh_pb2.AttributeMsg.NONE
        
        vertex_struct_string += '3f'

        attribute = mesh_buffer.Attribute.add()
        attribute.Semantic   = mesh_pb2.AttributeMsg.NORMAL
        attribute.Type       = mesh_pb2.AttributeMsg.FLOAT
        attribute.Size       = 3
        attribute.Flags      = mesh_pb2.AttributeMsg.NONE
        vertex_struct_string += '3f'

        if(len(mesh.uv_layers) > 0):

            mesh.calc_tangents(uvmap=mesh.uv_layers[0].name)

            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic   = mesh_pb2.AttributeMsg.TANGENT
            attribute.Type       = mesh_pb2.AttributeMsg.FLOAT
            attribute.Size       = 3
            attribute.Flags      = mesh_pb2.AttributeMsg.NONE
            vertex_struct_string += '3f'

            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic   = mesh_pb2.AttributeMsg.BITANGENT
            attribute.Type       = mesh_pb2.AttributeMsg.FLOAT
            attribute.Size       = 3
            attribute.Flags      = mesh_pb2.AttributeMsg.NONE
            vertex_struct_string += '3f'

            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic   = mesh_pb2.AttributeMsg.TEXCOORD
            attribute.Type       = mesh_pb2.AttributeMsg.FLOAT
            attribute.Size       = 2
            attribute.Flags      = mesh_pb2.AttributeMsg.NONE
            vertex_struct_string += '2f'

        # Weights are only included if there is an armature modifier.
        if self.armature is not None:
            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic   = mesh_pb2.AttributeMsg.WEIGHT_INDEX
            attribute.Type       = mesh_pb2.AttributeMsg.UNSIGNED_BYTE
            attribute.Size       = 4
            attribute.Flags      = mesh_pb2.AttributeMsg.INTEGER
            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic   = mesh_pb2.AttributeMsg.WEIGHT_VALUE
            attribute.Type       = mesh_pb2.AttributeMsg.UNSIGNED_BYTE
            attribute.Size       = 4
            attribute.Flags      = mesh_pb2.AttributeMsg.NORMALIZED
            vertex_struct_string += '8B'

        if(len(mesh.color_attributes) > 0):
            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic   = mesh_pb2.AttributeMsg.COLOR
            attribute.Type       = mesh_pb2.AttributeMsg.FLOAT
            attribute.Size       = 4
            attribute.Flags      = mesh_pb2.AttributeMsg.NONE
            vertex_struct_string += '3f'

        # Generate Vertex Buffer--------------------------------------
        self.vertices = list(pool.map(self.get_vertex, zip(mesh.loops, itertools.repeat(mesh_buffer.Attribute))))
        self.vertices.sort(key=operator.itemgetter(1))
        # The next line of code is so dense;
        # every single statement has so many things going on...
        self.vertices = sorted(map(self.get_indices_per_vertex, itertools.groupby(
            self.vertices, key=operator.itemgetter(1))), key=operator.itemgetter(0))
        # ... but it basically collects all indices that reference the same vertex
        # and packs them as a list of lists of lists like so:
        # [[[index,...],[vertex attribute values]],...]
        # Generate Index Buffer---------------------------------------
        polygon_count = 0
        index_dictionary = {}
        for entry in map(
            self.get_index_dictionary, self.vertices, range(
                0, len(
                self.vertices))):
            index_dictionary.update(entry)
        self.indices = []
        for polygon in mesh.polygons:
            polygon_count = polygon_count + 1
            # print("\rPolygon ", polygon_count, " of ", len(
            #    mesh.polygons))
            if(len(polygon.loop_indices) < 3):  # skip invalid faces
                continue
            for i in range(1, len(polygon.loop_indices), 2):
                self.indices.append(
                    index_dictionary[
                        polygon.loop_indices[
                            i - 1]])
                self.indices.append(index_dictionary[polygon.loop_indices[i]])
                self.indices.append(index_dictionary[polygon.loop_indices[
                                    (i + 1) % len(polygon.loop_indices)]])
        # Write vertices -----------------------------------
        vertex_struct = struct.Struct(vertex_struct_string)
        print(
            "Writing",
            len(self.vertices),
            "*",
            vertex_struct.size,
            "=",
            len(self.vertices) *
            vertex_struct.size,
            "bytes for vertex buffer")
        mesh_buffer.VertexCount = len(self.vertices)
        mesh_buffer.VertexBuffer = b''.join(
            list(pool.map(lambda x: vertex_struct.pack(*x[1]), self.vertices)))
        print("Done")

        index_struct = None
        # Write indices -----------------------------------
        # TODO: Allow for zero index meshes.
        mesh_buffer.IndexCount = len(self.indices)

        # Save memory space by using best fitting type for indices.
        if len(self.vertices) < 0x100:
            mesh_buffer.IndexSize = 1
            index_struct = struct.Struct('B')
        elif len(self.vertices) < 0x10000:
            mesh_buffer.IndexSize = 2
            index_struct = struct.Struct('H')
        else:
            mesh_buffer.IndexSize = 4
            index_struct = struct.Struct('I')

        print("Writting", mesh_buffer.IndexCount, "indices.")
        mesh_buffer.IndexBuffer = b''.join(
            list(pool.map(index_struct.pack, self.indices)))
        print("Done")
        pool.close()
        pool.join()

    def run(self, mesh_object):
        # Create Protocol Buffer
        mesh_buffer = mesh_pb2.MeshMsg()
        # Initialize Protocol Buffer Message
        mesh_buffer.Version = 1

        self.fill_mesh_buffer(mesh_buffer, mesh_object)
        # cProfile.runctx('self.fill_mesh_buffer(mesh_buffer, object)', globals(), locals())
        # print(
        # timeit.timeit(
        # lambda: self.fill_mesh_buffer(
        # mesh_buffer,
        # object),
        # number=1))

        # Open File for Writing
        print("Writting", self.filepath, ".")
        out = open(self.filepath, "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONMSH\x00'))
        out.write(mesh_buffer.SerializeToString())
        out.close()
        print("Done.")
        print("Writting", self.filepath.replace('.msh', '.txt'), ".")
        out = open(self.filepath.replace('.msh', '.txt'), "wt")
        out.write("AEONMSH\n")
        out.write(google.protobuf.text_format.MessageToString(mesh_buffer))
        out.close()
        print("Done.")


class MSH_OT_exporter(bpy.types.Operator):

    '''Exports a mesh to an AeonGames Mesh (MSH) file'''
    bl_idname = "export_mesh.msh"
    bl_label = "Export AeonGames Mesh"

    filepath: bpy.props.StringProperty(subtype='FILE_PATH')

    @classmethod
    def poll(cls, context):
        if (context.active_object.type == 'MESH'):
            return True
        return False

    def execute(self, context):
        if (context.active_object.type != 'MESH'):
            return {'CANCELLED'}
        bpy.ops.object.mode_set()
        self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        exporter = MSH_OT_exporterCommon(self.filepath)
        exporter.run(context.active_object)
        return {'FINISHED'}

    def invoke(self, context, event):
        if not self.filepath:
            self.filepath = bpy.path.ensure_ext(
                os.path.dirname(
                    bpy.data.filepath) +
                os.sep +
                context.active_object.name,
                ".msh")
        else:
            self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}


class MSHExportAll(bpy.types.Operator):

    '''Exports a Scene's meshes to a collection of AeonGames Mesh (MSH) files'''
    bl_idname = "export_mesh.all_msh"
    bl_label = "Export AeonGames Meshes"

    directory: bpy.props.StringProperty(subtype='DIR_PATH')

    @classmethod
    def poll(cls, context):
        return True

    def execute(self, context):
        for object in context.scene.objects:
            if (object.type == 'MESH'):
                exporter = MSH_OT_exporterCommon(
                    self.directory + os.sep + object.name + ".msh")
                exporter.run(object)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

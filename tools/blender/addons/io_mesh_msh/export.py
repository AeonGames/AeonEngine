# Copyright 2016 Rodrigo Jose Hernandez Cordoba
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
import math
import cProfile
import timeit
import mesh_pb2
import google.protobuf.text_format

ATTR_POSITION_MASK = 0b1
ATTR_NORMAL_MASK = 0b10
ATTR_TANGENT_MASK = 0b100
ATTR_BITANGENT_MASK = 0b1000
ATTR_UV_MASK = 0b10000
ATTR_WEIGHT_MASK = 0b100000

BYTE = 0x00
UNSIGNED_BYTE = 0x01
SHORT = 0x02
UNSIGNED_SHORT = 0x03
INT = 0x04
UNSIGNED_INT = 0x05
FLOAT = 0x06
TWO_BYTES = 0x07
THREE_BYTES = 0x08
FOUR_BYTES = 0x09
DOUBLE = 0x0A


class MSHExporter(bpy.types.Operator):
    '''Exports a mesh to an AeonGames Mesh (MSH) file'''
    bl_idname = "export_mesh.msh"
    bl_label = "Export AeonGames Mesh"

    filepath = bpy.props.StringProperty(subtype='FILE_PATH')

    @classmethod
    def poll(cls, context):
        return True

    def fill_triangle_group(self, triangle_group, mesh_object):
        # TODO: Store center radius instead (besides?) of min/max.
        triangle_group.Min.x = float('inf')
        triangle_group.Min.y = float('inf')
        triangle_group.Min.z = float('inf')
        triangle_group.Max.x = float('-inf')
        triangle_group.Max.y = float('-inf')
        triangle_group.Max.z = float('-inf')

        mesh_world_matrix = mathutils.Matrix(mesh_object.matrix_world)
        mesh = mesh_object.data
        mesh.calc_normals()

        # if this mesh is modified by an armature, find out which one.
        armature = None
        for modifier in mesh_object.modifiers:
            if modifier.type == 'ARMATURE':
                armaturemodifier = bpy.types.ArmatureModifier(modifier)
                if armaturemodifier.use_vertex_groups:
                    if armaturemodifier.object:
                        armature = armaturemodifier.object.data
                    break
        triangle_group.VertexFlags = 0
        vertex_struct_string = ''
        vertices = []
        index_buffer = []
        # Position and Normal aren't optional (for now)
        triangle_group.VertexFlags |= ATTR_POSITION_MASK
        vertex_struct_string += '3f'

        triangle_group.VertexFlags |= ATTR_NORMAL_MASK
        vertex_struct_string += '3f'

        if(len(mesh.uv_layers) > 0):

            mesh.calc_tangents(mesh.uv_layers[0].name)

            triangle_group.VertexFlags |= ATTR_TANGENT_MASK
            vertex_struct_string += '3f'

            triangle_group.VertexFlags |= ATTR_BITANGENT_MASK
            vertex_struct_string += '3f'

            triangle_group.VertexFlags |= ATTR_UV_MASK
            vertex_struct_string += '2f'

        # Weights are only included if there is an armature modifier.
        if armature is not None:
            triangle_group.VertexFlags |= ATTR_WEIGHT_MASK
            vertex_struct_string += '8B'

        # Generate Vertex Buffers--------------------------------------

        for polygon in mesh.polygons:
            if polygon.loop_total < 3:
                print("Invalid Face?")
                continue
            indices = []

            for loop_index in polygon.loop_indices:
                vertex = []
                # this should be a single function
                if triangle_group.VertexFlags & ATTR_POSITION_MASK:
                    localpos = mesh.vertices[
                        mesh.loops[loop_index].vertex_index].co * mesh_world_matrix

                    triangle_group.Min.x = min(
                        triangle_group.Min.x, localpos[0])
                    triangle_group.Min.y = min(
                        triangle_group.Min.y, localpos[1])
                    triangle_group.Min.z = min(
                        triangle_group.Min.z, localpos[2])
                    triangle_group.Max.x = max(
                        triangle_group.Max.x, localpos[0])
                    triangle_group.Max.y = max(
                        triangle_group.Max.y, localpos[1])
                    triangle_group.Max.z = max(
                        triangle_group.Max.z, localpos[2])

                    vertex.extend([localpos[0],
                                   localpos[1],
                                   localpos[2]])

                if triangle_group.VertexFlags & ATTR_NORMAL_MASK:
                    localnormal = mesh.vertices[
                        mesh.loops[loop_index].vertex_index].normal * mesh_world_matrix
                    vertex.extend([localnormal[0],
                                   localnormal[1],
                                   localnormal[2]])

                if triangle_group.VertexFlags & ATTR_TANGENT_MASK:
                    localtangent = mesh.loops[
                        loop_index].tangent * mesh_world_matrix
                    vertex.extend([localtangent[0],
                                   localtangent[1],
                                   localtangent[2]])

                if triangle_group.VertexFlags & ATTR_BITANGENT_MASK:
                    localbitangent = mesh.loops[
                        loop_index].bitangent * mesh_world_matrix
                    vertex.extend([localbitangent[0],
                                   localbitangent[1],
                                   localbitangent[2]])

                if triangle_group.VertexFlags & ATTR_UV_MASK:
                    vertex.extend([mesh.uv_layers[0].data[loop_index].uv[0],
                                   1.0 - mesh.uv_layers[0].data[loop_index].uv[1]])

                if triangle_group.VertexFlags & ATTR_WEIGHT_MASK:

                    weights = []

                    for group in mesh.vertices[
                            mesh.loops[loop_index].vertex_index].groups:
                        if group.weight > 0:
                            weights.append([group.weight, armature.bones.find(
                                mesh_object.vertex_groups[group.group].name)])

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

                if vertex not in vertices:
                    indices.append(len(vertices))
                    vertices.append(vertex)
                else:
                    indices.append(vertices.index(vertex))

            for i in range(1, len(indices), 2):
                index_buffer.append(indices[i - 1])
                index_buffer.append(indices[i])
                index_buffer.append(indices[(i + 1) % len(indices)])

        # Write vertices -----------------------------------
        vertex_struct = struct.Struct(vertex_struct_string)
        print(
            "Writing",
            len(vertices),
            "*",
            vertex_struct.size,
            "=",
            len(vertices) *
            vertex_struct.size,
            "bytes for vertex buffer")
        triangle_group.VertexCount = len(vertices)

        for vertex in vertices:
            triangle_group.VertexBuffer += vertex_struct.pack(*vertex)
        print("Done")

        index_struct = None
        # Write indices -----------------------------------
        triangle_group.IndexCount = len(index_buffer)

        # Save memory space by using best fitting type for indices.
        if len(vertices) < 0x100:
            triangle_group.IndexType = UNSIGNED_BYTE
            index_struct = struct.Struct('B')
        elif len(vertices) < 0x10000:
            triangle_group.IndexType = UNSIGNED_SHORT
            index_struct = struct.Struct('H')
        else:
            triangle_group.IndexType = UNSIGNED_INT
            index_struct = struct.Struct('I')
        print("Writting", triangle_group.IndexCount, "indices.")
        for index in index_buffer:
            triangle_group.IndexBuffer += index_struct.pack(index)
        print("Done")

    def execute(self, context):
        bpy.ops.object.mode_set()
        self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        # Create Protocol Buffer
        mesh_buffer = mesh_pb2.MeshBuffer()
        # Initialize Protocol Buffer Message
        mesh_buffer.Version = 1
        for object in context.scene.objects:
            if (object.type == 'MESH'):
                #self.fill_triangle_group(mesh_buffer.TriangleGroup.add(), object)
                #cProfile.runctx('self.fill_triangle_group(mesh_buffer.TriangleGroup.add(), object)', globals(), locals())
                print(
                    timeit.timeit(
                        lambda: self.fill_triangle_group(
                            mesh_buffer.TriangleGroup.add(),
                            object),
                        number=1))
        # Open File for Writing
        print("Writting", self.filepath, ".")
        out = open(self.filepath, "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONMSH\x00'))
        out.write(mesh_buffer.SerializeToString())
        out.close()
        print("Done.")
        print("Writting", self.filepath + ".txt", ".")
        out = open(self.filepath + ".txt", "wt")
        out.write("AEONMSH\n")
        out.write(google.protobuf.text_format.MessageToString(mesh_buffer))
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
                ".msh")
        else:
            self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

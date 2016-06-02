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

bl_info = {
    "name": "AeonGames Mesh Format (.msh)",
    "author": "Rodrigo Hernandez (Kwizatz)",
    "version": (1, 0, 0),
    "blender": (2, 7, 0),
    "location": "File > Export > Export AeonGames Mesh",
    "description": "Exports a mesh to an AeonGames Mesh (MSH) file",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

import bpy
import os
import struct
import mathutils
import math
import mesh_pb2

ATTR_POSITION_MASK  = 0b1
ATTR_NORMAL_MASK    = 0b10
ATTR_TANGENT_MASK   = 0b100
ATTR_BITANGENT_MASK = 0b1000
ATTR_UV_MASK        = 0b10000
ATTR_WEIGHT_MASK    = 0b100000

BYTE                  =         0x00
UNSIGNED_BYTE         =         0x01
SHORT                 =         0x02
UNSIGNED_SHORT        =         0x03
INT                   =         0x04
UNSIGNED_INT          =         0x05
FLOAT                 =         0x06
TWO_BYTES             =         0x07
THREE_BYTES           =         0x08
FOUR_BYTES            =         0x09
DOUBLE                =         0x0A

class MSHExporter(bpy.types.Operator):
    '''Exports a mesh to an AeonGames Mesh (MSH) file'''
    bl_idname = "export_mesh.msh"
    bl_label = "Export AeonGames Mesh"

    filepath = bpy.props.StringProperty(subtype='FILE_PATH')

    @classmethod
    def poll(cls, context):
        if (context.active_object.type=='MESH'):
            return True
        return False

    def execute(self, context):
        if (context.active_object.type!='MESH'):
            return {'CANCELLED'}

        bpy.ops.object.mode_set()
        self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        # Create Protocol Buffer
        mesh_buffer =  mesh_pb2.MeshBuffer()
        mesh_object = context.active_object
        mesh_world_matrix = mathutils.Matrix(mesh_object.matrix_world)
        mesh = context.active_object.data
        mesh.calc_normals()
        
        # if this mesh is modified by an armature, find out which one.
        armature = None
        for modifier in context.active_object.modifiers:
            if modifier.type == 'ARMATURE':
                armaturemodifier = bpy.types.ArmatureModifier(modifier)
                if armaturemodifier.use_vertex_groups:
                    armature = armaturemodifier.object.data
                    break
        mesh_buffer.VertexFlags = 0
        vertex_struct_string = ''
        vertices = []
        index_buffer = []
        # Position and Normal aren't optional (for now)
        mesh_buffer.VertexFlags |= ATTR_POSITION_MASK
        vertex_struct_string += '3f'
        
        mesh_buffer.VertexFlags |= ATTR_NORMAL_MASK
        vertex_struct_string += '3f'
                
        if(len(mesh.uv_layers)>0):
            mesh_buffer.VertexFlags |= ATTR_UV_MASK
            vertex_struct_string += '2f'

            mesh_buffer.VertexFlags |= ATTR_TANGENT_MASK
            vertex_struct_string += '3f'

            mesh_buffer.VertexFlags |= ATTR_BITANGENT_MASK
            vertex_struct_string += '3f'
            mesh.calc_tangents(mesh.uv_layers[0].name)
        
        # Weights are only included if there is an armature modifier.
        if armature is not None:
            mesh_buffer.VertexFlags |= ATTR_WEIGHT_MASK
            vertex_struct_string += '8B'

        # Generate Vertex Buffers-------------------------------------------------------------------------------------------------

        for polygon in mesh.polygons:
            if polygon.loop_total < 3:
                print("Invalid Face?")
                continue
            indices = []

            for loop_index in polygon.loop_indices:
                vertex = []
                # this should be a single function
                if mesh_buffer.VertexFlags & ATTR_POSITION_MASK:
                    localpos = mesh.vertices[mesh.loops[loop_index].vertex_index].co * mesh_world_matrix
                    vertex.extend([ localpos[0],
                                    localpos[1],
                                    localpos[2] ])

                if mesh_buffer.VertexFlags & ATTR_NORMAL_MASK:
                    localnormal = mesh.vertices[mesh.loops[loop_index].vertex_index].normal * mesh_world_matrix
                    vertex.extend([ localnormal[0],
                                    localnormal[1],
                                    localnormal[2] ])
                                    
                if mesh_buffer.VertexFlags & ATTR_TANGENT_MASK:
                    localtangent = mesh.vertices[mesh.loops[loop_index].vertex_index].tangent * mesh_world_matrix
                    vertex.extend([ localtangent[0],
                                    localtangent[1],
                                    localtangent[2] ])

                if mesh_buffer.VertexFlags & ATTR_BITANGENT_MASK:
                    localbitangent = mesh.vertices[mesh.loops[loop_index].vertex_index].bitangent * mesh_world_matrix
                    vertex.extend([ localbitangent[0],
                                    localbitangent[1],
                                    localbitangent[2] ])
                                    
                if mesh_buffer.VertexFlags & ATTR_WEIGHT_MASK:

                    weights = []

                    for group in mesh.vertices[mesh.loops[loop_index].vertex_index].groups:
                        if  group.weight > 0:
                            weights.append([group.weight,armature.bones.find(mesh_object.vertex_groups[group.group].name)])

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
                        weight[0] = weight[0]/magnitude
                        weight[0] = int(weight[0]*0xFF)
                        byte_magnitude = byte_magnitude+weight[0]

                    weights[-1][0] = weights[-1][0] + (0xFF-byte_magnitude)

                    if len(weights) < 4:
                        weights = weights + [[0,0]]*(4 - len(weights))

                    weight_indices = []
                    weight_values  = []
                    weights.sort()
                    weights.reverse()
                    for weight in weights:
                        weight_indices.append(weight[1])
                        weight_values.append( weight[0])
                    vertex.extend(weight_indices)
                    vertex.extend(weight_values)

                if mesh_buffer.VertexFlags & ATTR_UV_MASK:
                    vertex.extend([ mesh.uv_layers[0].data[loop_index].uv[0],
                                    1.0-mesh.uv_layers[0].data[loop_index].uv[1] ])
                                    
                if vertex not in vertices:
                    indices.append(len(vertices))
                    vertices.append(vertex)
                else:
                    indices.append(vertices.index(vertex))

            for i in range(1,len(indices),2):
                index_buffer.append(indices[i-1])
                index_buffer.append(indices[i])
                index_buffer.append(indices[(i+1) % len(indices)])

        
        # Fill Protocol Buffer ----------------------------------------------------------
        mesh_buffer.Version = 1
        mesh_buffer.Min.x = float('inf')
        mesh_buffer.Min.y = float('inf')
        mesh_buffer.Min.z = float('inf')
        mesh_buffer.Max.x = float('-inf')
        mesh_buffer.Max.y = float('-inf')
        mesh_buffer.Max.z = float('-inf')
        mesh_buffer.VertexFlags = mesh_buffer.VertexFlags

        # Write vertices -----------------------------------
        vertex_struct = struct.Struct(vertex_struct_string)
        print("Writing",len(vertices),"*",vertex_struct.size,"=",len(vertices) * vertex_struct.size,"bytes for vertex buffer")
        mesh_buffer.VertexCount = len(vertices)

        for vertex in vertices:
            mesh_buffer.VertexBuffer += vertex_struct.pack(*vertex)

        index_struct = None
        # Write indices -----------------------------------
        mesh_buffer.IndexCount = len(index_buffer)

        # Save memory space by using best fitting type for indices.
        if len(vertices) < 0x100:
            mesh_buffer.IndexType = UNSIGNED_BYTE
            index_struct = struct.Struct('B')
        elif len(vertices) < 0x10000:
            mesh_buffer.IndexType = UNSIGNED_SHORT
            index_struct = struct.Struct('H')
        else:
            mesh_buffer.IndexType = UNSIGNED_INT
            index_struct = struct.Struct('I')
        print("Writting",mesh_buffer.IndexCount,"indices.")
        for index in index_buffer:
            mesh_buffer.VertexBuffer += index_struct.pack(index)
        #------------------------------------------------
        # Open File for Writing ----------------------------------------------------------
        out = open(self.filepath, "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONMSH\x00'))
        out.write(mesh_buffer.SerializeToString())
        out.close()
        return {'FINISHED'}

    def invoke(self, context, event):
        if not self.filepath:
            self.filepath = bpy.path.ensure_ext(os.path.dirname(bpy.data.filepath)+os.sep+context.active_object.name, ".msh")
        else:
            self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

def msh_menu_func(self, context):
    self.layout.operator(MSHExporter.bl_idname, text="AeonGames Mesh (.msh)")

def register():
    bpy.utils.register_class(MSHExporter)
    bpy.types.INFO_MT_file_export.append(msh_menu_func)

if __name__ == "__main__":
    register()
# Copyright (C) 2016-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba
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
import bmesh
import struct
import mathutils
import mesh_pb2
import operator
import itertools
import google.protobuf.text_format
from multiprocessing import Pool
from multiprocessing.dummy import Pool as ThreadPool, Lock as ThreadLock


class MSH_OT_exporterCommon():

    def __init__(self, filepath, export_tangents=True, export_uvs=True, export_weights=True, export_colors=True, as_text=False):
        self.mesh = None
        self.object = None
        self.armature = None
        self.vertices = []
        self.indices = []
        self.filepath = filepath
        self.export_tangents = export_tangents
        self.export_uvs = export_uvs
        self.export_weights = export_weights
        self.export_colors = export_colors
        self.as_text = as_text

    def get_vertex(self, loop_and_attributes):
        loop = loop_and_attributes[0]
        attributes = loop_and_attributes[1]
        vertex = []

        for attribute in attributes:
            if attribute.Semantic == mesh_pb2.AttributeMsg.POSITION:
                # Points carry translation, so transform by the full matrix.
                position = self.world_matrix @ self.mesh.vertices[loop.vertex_index].co
                vertex.extend([position.x, position.y, position.z])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.NORMAL:
                # Use the per-loop (split) normal so hard edges are preserved,
                # transformed by the inverse-transpose to survive non-uniform scale.
                normal = (self.normal_matrix @ loop.normal).normalized()
                vertex.extend([normal.x, normal.y, normal.z])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.TANGENT:
                # Real per-loop tangent from calc_tangents, rotated into world space.
                tangent = (self.world_3x3 @ loop.tangent).normalized()
                vertex.extend([tangent.x, tangent.y, tangent.z])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.BITANGENT:
                # Real per-loop bitangent from calc_tangents, rotated into world space.
                bitangent = (self.world_3x3 @ loop.bitangent).normalized()
                vertex.extend([bitangent.x, bitangent.y, bitangent.z])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.TEXCOORD:
                vertex.extend([self.mesh.uv_layers[0].data[loop.index].uv[0],
                            1.0 - self.mesh.uv_layers[0].data[loop.index].uv[1]])
            elif attribute.Semantic == mesh_pb2.AttributeMsg.WEIGHT_INDEX:
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

    def fill_mesh_buffer(self, mesh_buffer, mesh_object, material_index=None):
        # Warn if the source mesh contains n-gons: calc_tangents() only accepts
        # tris/quads, so we triangulate below to keep the export working, but
        # the user gets cleaner, more predictable results by triangulating or
        # quadrangulating the mesh themselves before export.
        if any(len(polygon.vertices) > 4 for polygon in mesh_object.data.polygons):
            print(
                "Warning: mesh '" + mesh_object.name + "' contains n-gons; "
                "triangulating on export. Consider triangulating or "
                "quadrangulating the mesh before exporting.")
        # Work on a triangulated copy of the mesh: calc_tangents() only accepts
        # tris/quads, so n-gons (common in Sponza) would otherwise abort the
        # export. Triangulating a copy leaves the user's original data intact.
        mesh = mesh_object.data.copy()
        bm = bmesh.new()
        bm.from_mesh(mesh)
        bmesh.ops.triangulate(bm, faces=bm.faces)
        bm.to_mesh(mesh)
        bm.free()
        self.mesh = mesh
        self.object = mesh_object
        # Cache the transforms used to take vertex attributes into world space:
        # points by the full matrix, tangents/bitangents by its 3x3 rotation,
        # and normals by the inverse-transpose so non-uniform scale is handled.
        self.world_matrix = mesh_object.matrix_world.copy()
        self.world_3x3 = self.world_matrix.to_3x3()
        self.normal_matrix = self.world_3x3.inverted_safe().transposed()
        pool = ThreadPool()

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
        attribute.Semantic = mesh_pb2.AttributeMsg.POSITION
        attribute.Type = mesh_pb2.AttributeMsg.FLOAT
        attribute.Size = 3
        attribute.Flags = mesh_pb2.AttributeMsg.NONE

        vertex_struct_string += '3f'

        attribute = mesh_buffer.Attribute.add()
        attribute.Semantic = mesh_pb2.AttributeMsg.NORMAL
        attribute.Type = mesh_pb2.AttributeMsg.FLOAT
        attribute.Size = 3
        attribute.Flags = mesh_pb2.AttributeMsg.NONE
        vertex_struct_string += '3f'

        if(len(mesh.uv_layers) > 0):

            mesh.calc_tangents(uvmap=mesh.uv_layers[0].name)

            if self.export_tangents:
                attribute = mesh_buffer.Attribute.add()
                attribute.Semantic = mesh_pb2.AttributeMsg.TANGENT
                attribute.Type = mesh_pb2.AttributeMsg.FLOAT
                attribute.Size = 3
                attribute.Flags = mesh_pb2.AttributeMsg.NONE
                vertex_struct_string += '3f'

                attribute = mesh_buffer.Attribute.add()
                attribute.Semantic = mesh_pb2.AttributeMsg.BITANGENT
                attribute.Type = mesh_pb2.AttributeMsg.FLOAT
                attribute.Size = 3
                attribute.Flags = mesh_pb2.AttributeMsg.NONE
                vertex_struct_string += '3f'

            if self.export_uvs:
                attribute = mesh_buffer.Attribute.add()
                attribute.Semantic = mesh_pb2.AttributeMsg.TEXCOORD
                attribute.Type = mesh_pb2.AttributeMsg.FLOAT
                attribute.Size = 2
                attribute.Flags = mesh_pb2.AttributeMsg.NONE
                vertex_struct_string += '2f'

        # Weights are only included if there is an armature modifier and export_weights is True.
        if self.armature is not None and self.export_weights:
            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic = mesh_pb2.AttributeMsg.WEIGHT_INDEX
            attribute.Type = mesh_pb2.AttributeMsg.UNSIGNED_BYTE
            attribute.Size = 4
            attribute.Flags = mesh_pb2.AttributeMsg.INTEGER
            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic = mesh_pb2.AttributeMsg.WEIGHT_VALUE
            attribute.Type = mesh_pb2.AttributeMsg.UNSIGNED_BYTE
            attribute.Size = 4
            attribute.Flags = mesh_pb2.AttributeMsg.NORMALIZED
            vertex_struct_string += '8B'

        if(len(mesh.color_attributes) > 0 and self.export_colors):
            attribute = mesh_buffer.Attribute.add()
            attribute.Semantic = mesh_pb2.AttributeMsg.COLOR
            attribute.Type = mesh_pb2.AttributeMsg.FLOAT
            attribute.Size = 4
            attribute.Flags = mesh_pb2.AttributeMsg.NONE
            vertex_struct_string += '3f'

        # Restrict to the polygons that reference the requested material slot
        # when a split was asked for; otherwise process the whole mesh.
        if material_index is None:
            polygons = mesh.polygons
        else:
            polygons = [p for p in mesh.polygons if p.material_index == material_index]
        loops = [mesh.loops[loop_index]
                 for polygon in polygons for loop_index in polygon.loop_indices]

        # Generate Vertex Buffer--------------------------------------
        self.vertices = list(pool.map(self.get_vertex, zip(loops, itertools.repeat(mesh_buffer.Attribute))))
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
        for polygon in polygons:
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
        # Compute the bounding sphere from the generated world-space positions.
        # POSITION is always the first attribute, so it occupies the first three
        # floats of every vertex; deriving the bounds here keeps them consistent
        # with the exported (world-space) geometry and any per-material split.
        if self.vertices:
            xs = [vertex[1][0] for vertex in self.vertices]
            ys = [vertex[1][1] for vertex in self.vertices]
            zs = [vertex[1][2] for vertex in self.vertices]
            mesh_buffer.Center.x = (min(xs) + max(xs)) / 2
            mesh_buffer.Center.y = (min(ys) + max(ys)) / 2
            mesh_buffer.Center.z = (min(zs) + max(zs)) / 2
            mesh_buffer.Radii.x = max(xs) - mesh_buffer.Center.x
            mesh_buffer.Radii.y = max(ys) - mesh_buffer.Center.y
            mesh_buffer.Radii.z = max(zs) - mesh_buffer.Center.z

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

        # Write indices -----------------------------------
        # Only write an index buffer if it provides vertex deduplication benefit.
        if len(self.vertices) < len(self.indices):
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

            print("Writing", mesh_buffer.IndexCount, "indices.")
            mesh_buffer.IndexBuffer = b''.join(
                list(pool.map(index_struct.pack, self.indices)))
            print("Done")
        else:
            # No index buffer: expand vertices into triangle-draw order
            # so the GPU can render them sequentially.
            print("Mesh does not benefit from an index buffer, expanding vertices.")
            expanded_vertices = [self.vertices[i] for i in self.indices]
            mesh_buffer.VertexCount = len(expanded_vertices)
            mesh_buffer.VertexBuffer = b''.join(
                list(pool.map(lambda x: vertex_struct.pack(*x[1]), expanded_vertices)))
            mesh_buffer.IndexCount = 0
            mesh_buffer.IndexSize = 0
            print("Done")
        pool.close()
        pool.join()
        # Release the temporary triangulated mesh datablock created above.
        bpy.data.meshes.remove(mesh)

    def run(self, mesh_object, material_index=None):
        # Create Protocol Buffer
        mesh_buffer = mesh_pb2.MeshMsg()
        # Initialize Protocol Buffer Message
        mesh_buffer.Version = 1

        self.fill_mesh_buffer(mesh_buffer, mesh_object, material_index)
        # cProfile.runctx('self.fill_mesh_buffer(mesh_buffer, object)', globals(), locals())
        # print(
        # timeit.timeit(
        # lambda: self.fill_mesh_buffer(
        # mesh_buffer,
        # object),
        # number=1))

        # Open File for Writing
        if self.as_text:
            text_path = self.filepath.replace('.msh', '.txt')
            print("Writting", text_path, ".")
            out = open(text_path, "wt")
            out.write("AEONMSH\n")
            out.write(google.protobuf.text_format.MessageToString(mesh_buffer))
            out.close()
            print("Done.")
        else:
            print("Writting", self.filepath, ".")
            out = open(self.filepath, "wb")
            magick_struct = struct.Struct('8s')
            out.write(magick_struct.pack(b'AEONMSH\x00'))
            out.write(mesh_buffer.SerializeToString())
            out.close()
            print("Done.")


class MSH_OT_exporter(bpy.types.Operator):

    '''Exports a mesh to an AeonGames Mesh (MSH) file'''
    bl_idname = "export_mesh.msh"
    bl_label = "Export AeonGames Mesh"

    filepath: bpy.props.StringProperty(subtype='FILE_PATH')
    
    # Export options (Position and Normal are always exported)
    export_tangents: bpy.props.BoolProperty(
        name="Export Tangents",
        description="Export tangent and bitangent vectors (requires UVs)",
        default=True
    )
    export_uvs: bpy.props.BoolProperty(
        name="Export UVs",
        description="Export texture coordinates",
        default=True
    )
    export_weights: bpy.props.BoolProperty(
        name="Export Weights",
        description="Export vertex weights for skeletal animation",
        default=True
    )
    export_colors: bpy.props.BoolProperty(
        name="Export Vertex Colors",
        description="Export vertex color attributes",
        default=True
    )
    as_text: bpy.props.BoolProperty(
        name="Export as Text",
        description="Write the protobuf message as a human-readable text file (.txt) instead of the binary .msh",
        default=False
    )
    material_index: bpy.props.IntProperty(
        name="Material Index",
        description="Only export polygons assigned to this material slot; -1 exports the whole mesh",
        default=-1
    )

    @classmethod
    def poll(cls, context):
        return context.active_object is not None and context.active_object.type == 'MESH'

    def execute(self, context):
        if context.active_object is None or context.active_object.type != 'MESH':
            return {'CANCELLED'}
        bpy.ops.object.mode_set()
        self.filepath = bpy.path.ensure_ext(self.filepath, ".msh")
        exporter = MSH_OT_exporterCommon(
            self.filepath,
            export_tangents=self.export_tangents,
            export_uvs=self.export_uvs,
            export_weights=self.export_weights,
            export_colors=self.export_colors,
            as_text=self.as_text
        )
        material_index = None if self.material_index < 0 else self.material_index
        exporter.run(context.active_object, material_index)
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
    
    # Export options (Position and Normal are always exported)
    export_tangents: bpy.props.BoolProperty(
        name="Export Tangents",
        description="Export tangent and bitangent vectors (requires UVs)",
        default=True
    )
    export_uvs: bpy.props.BoolProperty(
        name="Export UVs",
        description="Export texture coordinates",
        default=True
    )
    export_weights: bpy.props.BoolProperty(
        name="Export Weights",
        description="Export vertex weights for skeletal animation",
        default=True
    )
    export_colors: bpy.props.BoolProperty(
        name="Export Vertex Colors",
        description="Export vertex color attributes",
        default=True
    )
    as_text: bpy.props.BoolProperty(
        name="Export as Text",
        description="Write the protobuf message as a human-readable text file (.txt) instead of the binary .msh",
        default=False
    )

    @classmethod
    def poll(cls, context):
        return True

    def execute(self, context):
        for object in context.scene.objects:
            if (object.type == 'MESH'):
                exporter = MSH_OT_exporterCommon(
                    self.directory + os.sep + object.name + ".msh",
                    export_tangents=self.export_tangents,
                    export_uvs=self.export_uvs,
                    export_weights=self.export_weights,
                    export_colors=self.export_colors,
                    as_text=self.as_text
                )
                exporter.run(object)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

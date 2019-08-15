# Copyright (C) 2012,2013,2015,2017,2019 Rodrigo Jose Hernandez Cordoba
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
import time  # used to measure time taken during execution
import itertools
import geometry_pb2
import google.protobuf.text_format

class CLN_OT_exporter(bpy.types.Operator):
    '''Exports a mesh as AeonGames Collision Data file (CLN) file'''
    bl_idname = "export_mesh.cln"
    bl_label = "Export AeonGames Collision Data"
    filepath: bpy.props.StringProperty(subtype='FILE_PATH')

    def __init__(self):
        self.index_struct = struct.Struct('i')

    @classmethod
    def poll(cls, context):
        # Export from any context state?
        return True

    def addplane(self, plane, geometry_buffer):
        i = 0
        for iplane in geometry_buffer.plane:
            if ((math.fabs(plane[0] - iplane.x) <= 0.00001) and
                (math.fabs(plane[1] - iplane.y) <= 0.00001) and
                (math.fabs(plane[2] - iplane.z) <= 0.00001) and
                    (math.fabs(plane[3] - iplane.w) <= 0.00001)):
                #print("Exact Match",plane,iplane)
                return i

            if ((math.fabs(-plane[0] - iplane.x) <= 0.00001) and
                (math.fabs(-plane[1] - iplane.y) <= 0.00001) and
                (math.fabs(-plane[2] - iplane.z) <= 0.00001) and
                    (math.fabs(-plane[3] - iplane.w) <= 0.00001)):
                #print("Flipped Plane")
                return -(i + 1)

            i = i + 1
        plane_buffer = geometry_buffer.plane.add()
        plane_buffer.x = plane[0]
        plane_buffer.y = plane[1]
        plane_buffer.z = plane[2]
        plane_buffer.w = plane[3]
        return len(geometry_buffer.plane) - 1

    def calculate_sixdops(self, mesh, geometry_buffer, collision_faces):
        count = 1
        for polygon in mesh.polygons:
            #print("Processing polygon ", count, " out of ", len(mesh.polygons))
            count = count + 1
            # Calculate polygon 6DOP
            sixdop = [
                float('-inf'),
                float('-inf'),
                float('-inf'),
                float('inf'),
                float('inf'),
                float('inf')
            ]
            for index in polygon.vertices:
                # TODO: Apply transforms before calculating sixdop.
                sixdop[0] = (max(mesh.vertices[index].co[0], sixdop[0]))
                sixdop[1] = (max(mesh.vertices[index].co[1], sixdop[1]))
                sixdop[2] = (max(mesh.vertices[index].co[2], sixdop[2]))
                sixdop[3] = (min(mesh.vertices[index].co[0], sixdop[3]))
                sixdop[4] = (min(mesh.vertices[index].co[1], sixdop[4]))
                sixdop[5] = (min(mesh.vertices[index].co[2], sixdop[5]))
            # Adjust distance for normal direction, otherwise this is just
            # min-max
            sixdop[3] *= -1.0  # (sixdop[3],0,0) dot (-1,0,0)
            sixdop[4] *= -1.0  # (sixdop[4],0,0) dot (0,-1,0)
            sixdop[5] *= -1.0  # (sixdop[5],0,0) dot (0,0,-1)
            collision_faces[polygon] = geometry_buffer.brush.add()
            collision_faces[polygon].six_dop.positive.x = sixdop[0]
            collision_faces[polygon].six_dop.positive.y = sixdop[1]
            collision_faces[polygon].six_dop.positive.z = sixdop[2]
            collision_faces[polygon].six_dop.negative.x = sixdop[3]
            collision_faces[polygon].six_dop.negative.y = sixdop[4]
            collision_faces[polygon].six_dop.negative.z = sixdop[5]

            # Calculate face plane if face normal does not match a 6DOP
            if not (
                math.fabs(
                    polygon.normal[0]) >= 0.999999 and math.fabs(
                    polygon.normal[0]) <= 1.000001) and not (
                math.fabs(
                    polygon.normal[1]) >= 0.999999 and math.fabs(
                        polygon.normal[1]) <= 1.000001) and not (
                            math.fabs(
                                polygon.normal[2]) >= 0.999999 and math.fabs(
                                    polygon.normal[2]) <= 1.000001):
                plane = [polygon.normal[0], polygon.normal[1], polygon.normal[2],
                         polygon.normal.dot(mesh.vertices[polygon.vertices[0]].co)]
                collision_faces[polygon].plane_indices += CLN_OT_exporter.index_struct.pack(
                    self.addplane(plane, geometry_buffer))
            # else:
            #    print("Polygon plane matches a 6-DOP, discarded")

    def calculate_edgeplanes(self, mesh, geometry_buffer, collision_faces):
        # Calculate edge planes, must take neighbouring polygon into account
        # (if any)
        count = 1
        for edge in mesh.edges:
            #print("Processing edge ", count, " out of ", len(mesh.edges))
            count = count + 1
            adjacent_faces = []
            for polygon in mesh.polygons:
                if edge.key in polygon.edge_keys:
                    adjacent_faces.append(polygon)
            if len(adjacent_faces) == 1:
                # edge is boundary
                edgevector = mesh.vertices[edge.vertices[0]
                                           ].co - mesh.vertices[edge.vertices[1]].co
                edgevector.normalize()
                normal = adjacent_faces[0].normal.cross(edgevector)
                normal.normalize()
                distance = normal.dot(mesh.vertices[edge.vertices[0]].co)

                # If the face is in front of the plane, flip the plane
                for vertex in adjacent_faces[0].vertices:
                    if (edge.vertices[0] == vertex) or (
                            edge.vertices[1] == vertex):
                        continue
                    elif (mesh.vertices[vertex].co[0] * normal[0] + mesh.vertices[vertex].co[1] * normal[1] + mesh.vertices[vertex].co[2] * normal[2] - distance) >= 0.000001:
                        # plane is pointing in the wrong direction, flip
                        normal *= -1
                        distance *= -1
                        break

                plane_index = self.addplane(
                    [normal[0], normal[1], normal[2], distance], geometry_buffer)
                collision_faces[adjacent_faces[0]
                                ].plane_indices += CLN_OT_exporter.index_struct.pack(plane_index)
                # TODO: Add extra bevelling plane to edge using face normal and
                # the normal of the created plane.
            elif len(adjacent_faces) == 2:
                # edge is manifold (shared by two faces)
                # -- This code should work for faces with an angle >= 180 degrees --
                # -- between them (Reflex angles).
                normal = adjacent_faces[0].normal + adjacent_faces[1].normal
                normal.normalize()
                distance = normal.dot(mesh.vertices[edge.vertices[0]].co)

                # Both faces should be behind the plane, otherwise this plane
                # should be skipped.
                for vertex in adjacent_faces[0].vertices:
                    if (edge.vertices[0] == vertex) or (
                            edge.vertices[1] == vertex):
                        continue
                    elif (normal.dot(mesh.vertices[vertex].co) - distance) <= 0.000001:
                        #print ("Convex Edge")
                        plane_index = self.addplane(
                            [normal[0], normal[1], normal[2], distance], geometry_buffer)
                        # Same plane for both faces.
                        collision_faces[adjacent_faces[0]
                                        ].plane_indices += CLN_OT_exporter.index_struct.pack(plane_index)
                        collision_faces[adjacent_faces[1]
                                        ].plane_indices += CLN_OT_exporter.index_struct.pack(plane_index)
                        edgedir = mesh.vertices[edge.vertices[0]
                                                ].co - mesh.vertices[edge.vertices[1]].co
                        edgedir.normalize()
                        normal = normal.cross(edgedir)
                        normal.normalize()
                        distance = normal.dot(
                            mesh.vertices[edge.vertices[0]].co)
                        # Find out which face is in front of the plane.
                        for vertex in adjacent_faces[0].vertices:
                            if (edge.vertices[0] == vertex) or (
                                    edge.vertices[1] == vertex):
                                continue
                            elif (mesh.vertices[vertex].co[0] * normal[0] + mesh.vertices[vertex].co[1] * normal[1] + mesh.vertices[vertex].co[2] * normal[2] - distance) >= 0.000001:
                                # face 0 is in front of the plane, switch with
                                # face 1
                                adjacent_faces.reverse()
                                break
                        plane_index = self.addplane(
                            [normal[0], normal[1], normal[2], distance], geometry_buffer)
                        collision_faces[adjacent_faces[0]
                                        ].plane_indices += CLN_OT_exporter.index_struct.pack(plane_index)
                        collision_faces[adjacent_faces[1]
                                        ].plane_indices += CLN_OT_exporter.index_struct.pack(-(plane_index + 1))
                        break
                    else:
                        #print ("Concave Edge")
                        # The planes cut each other in just the correct angles,
                        # but with their normals flipped
                        # so make a face plane be the edge plane of the other
                        # and vice-versa.
                        plane_index = self.addplane([
                            adjacent_faces[0].normal[0],
                            adjacent_faces[0].normal[1],
                            adjacent_faces[0].normal[2],
                            adjacent_faces[0].normal.dot(mesh.vertices[adjacent_faces[0].vertices[0]].co)], geometry_buffer)
                        collision_faces[adjacent_faces[1]
                                        ].plane_indices += CLN_OT_exporter.index_struct.pack(-(plane_index + 1))
                        plane_index = self.addplane([
                            adjacent_faces[1].normal[0],
                            adjacent_faces[1].normal[1],
                            adjacent_faces[1].normal[2],
                            adjacent_faces[1].normal.dot(mesh.vertices[adjacent_faces[1].vertices[0]].co)], geometry_buffer)
                        collision_faces[adjacent_faces[0]
                                        ].plane_indices += CLN_OT_exporter.index_struct.pack(-(plane_index + 1))
                        break
            else:
                # either edge has no faces attached or has more than 2, either
                # way it is invalid for collision detection.
                edge.select = True
                print("Non-Manifold Edge")

    def calculate_vertexplanes(self, mesh, geometry_buffer, collision_faces):
        # Calculate vertex planes.
        count = 1
        for vertex in mesh.vertices:
            #print("Processing vertex ", count, " out of ", len(mesh.vertices))
            count = count + 1
            distance = vertex.normal.dot(vertex.co)
            # TODO: plane must only be added if the vertex is "sharp".
            plane_index = self.addplane([
                vertex.normal[0],
                vertex.normal[1],
                vertex.normal[2],
                distance], geometry_buffer)
            for polygon in mesh.polygons:
                if vertex in polygon.vertices:
                    collision_faces[polygon].plane_indices += CLN_OT_exporter.index_struct.pack(
                        plane_index)

    def process_collision_mesh(self, mesh_object, geometry_buffer):
        #print("Processing Mesh ", mesh_object.name, " for collision data.")
        mesh = mesh_object.data
        collision_faces = {}

        self.calculate_sixdops(mesh, geometry_buffer, collision_faces)
        self.calculate_edgeplanes(mesh, geometry_buffer, collision_faces)
        self.calculate_vertexplanes(mesh, geometry_buffer, collision_faces)

    def execute(self, context):
        seconds = time.time()
        bpy.ops.object.mode_set()
        scene = context.scene
        self.filepath = bpy.path.ensure_ext(self.filepath, ".cln")
        geometry_buffer = geometry_pb2.GeometryBuffer()
        for obj in scene.objects:
            if (obj.type == 'MESH'):
                self.process_collision_mesh(obj, geometry_buffer)

        # Open File for Writing
        print("Writting", self.filepath, ".")
        out = open(self.filepath, "wb")
        magick_struct = struct.Struct('8s')
        out.write(magick_struct.pack(b'AEONCLN\x00'))
        out.write(geometry_buffer.SerializeToString())
        out.close()
        print("Done.")

        print("Writting", self.filepath.replace('.cln', '.txt'), ".")
        for brush in geometry_buffer.brush:
            fmt = ""
            for i in range(0, len(brush.plane_indices) //
                           struct.calcsize('i')):
                fmt += 'i'
            plane_indices = ""
            for i in struct.unpack(fmt, brush.plane_indices):
                plane_indices += str(i) + " "
            brush.plane_indices = bytes(plane_indices.strip(), 'utf-8')
        out = open(self.filepath.replace('.cln', '.txt'), "wt")
        out.write("AEONCLN\n")
        out.write(google.protobuf.text_format.MessageToString(geometry_buffer))
        out.close()
        print("Done.")

        seconds = time.time() - seconds
        print("Export took ", seconds, " seconds.")
        return {'FINISHED'}

    def invoke(self, context, event):
        if not self.filepath:
            self.filepath = bpy.path.ensure_ext(
                os.path.dirname(
                    bpy.data.filepath) +
                os.sep +
                context.scene.name,
                ".cln")
        else:
            self.filepath = bpy.path.ensure_ext(self.filepath, ".cln")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

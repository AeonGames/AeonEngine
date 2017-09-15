# Copyright (C) 2012,2013,2015,2017 Rodrigo Jose Hernandez Cordoba
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
import animation_pb2
import google.protobuf.text_format

class GTYKdNode():
    kdnode_struct = struct.Struct('Ifii')
    def __init__(self):
        self.axis = 0
        self.distance = 0
        self.near_index = 0
        self.far_index = 0
        self.near = None
        self.far = None
    def write(self, file):
        file.write(
            GTYKdNode.kdnode_struct.pack(
                self.axis,
                self.distance,
                self.near_index,
                self.far_index))

class GTYKdLeaf():
    kdleaf_struct = struct.Struct('II')
    kdleaf_index_struct = struct.Struct('I')

    def __init__(self):
        self.brush_indices = []
        self.count = 0
        self.offset = 0

    def write_brush_indices(self, file):
        self.count = len(self.brush_indices)
        if self.count == 1:
            self.offset = self.brush_indices[0]
        else:
            self.offset = file.tell()
            for index in self.brush_indices:
                file.write(GTYKdLeaf.kdleaf_index_struct.pack(index))

    def write(self, file):
        file.write(GTYKdLeaf.kdleaf_struct.pack(self.count, self.offset))


class GTYBrush():
    brush_struct = struct.Struct('6f2I')

    def __init__(self, sixdop, brush_plane_start, brush_plane_count):
        self.sixdop = sixdop
        self.brush_plane_start = brush_plane_start
        self.brush_plane_count = brush_plane_count

    def write(self, file):
        file.write(GTYBrush.brush_struct.pack(self.sixdop[0],
                                              self.sixdop[1],
                                              self.sixdop[2],
                                              self.sixdop[3],
                                              self.sixdop[4],
                                              self.sixdop[5],
                                              self.brush_plane_start,
                                              self.brush_plane_count))


class GTYExporter(bpy.types.Operator):
    '''Exports scene to an AeonGames Tile (GTY) file'''
    bl_idname = "export_scene.gty"
    bl_label = "Export AeonGames Level Geometry"

    filepath = bpy.props.StringProperty(subtype='FILE_PATH')

    def __init__(self):
        # Collision Data
        self.planes = []  # Collision
        self.plane_indices = []  # Collision
        self.brushes = []  # Polygon Brushes
        self.kdpoints = []
        self.kdtreenodes = []
        self.kdtreeleaves = []
        # Visual Data
        self.materials = {}
        self.string_table = commoncode.StringTable()

    @classmethod
    def poll(cls, context):
        # Export from any context state?
        return True

    def addplane(self, plane):
        #print("Adding Plane",plane)
        for iplane in self.planes:
            if ((math.fabs(plane[0] - iplane[0]) <= 0.00001) and
                (math.fabs(plane[1] - iplane[1]) <= 0.00001) and
                (math.fabs(plane[2] - iplane[2]) <= 0.00001) and
                    (math.fabs(plane[3] - iplane[3]) <= 0.00001)):
                #print("Exact Match",plane,iplane)
                return self.planes.index(iplane)

            elif ((math.fabs(-plane[0] - iplane[0]) <= 0.00001) and
                  (math.fabs(-plane[1] - iplane[1]) <= 0.00001) and
                  (math.fabs(-plane[2] - iplane[2]) <= 0.00001) and
                  (math.fabs(-plane[3] - iplane[3]) <= 0.00001)):
                #print("Flipped Plane")
                return -(self.planes.index(iplane) + 1)

        self.planes.append(plane)
        return self.planes.index(plane)

    def process_kd_mesh(self, mesh):
        for vertex in mesh.vertices:
            if vertex.co not in self.kdpoints:
                self.kdpoints.append(vertex.co)

    def build_kd_tree(self, points, axes, axis_index):
        if not points:
            leaf = GTYKdLeaf()
            self.kdtreeleaves.append(leaf)
            return leaf
        points.sort(key=lambda point: point[axes[axis_index]])
        median = len(points) // 2
        node = GTYKdNode()
        self.kdtreenodes.append(node)
        node.axis = axes[axis_index]
        node.distance = points[median][node.axis]
        next_axis_index = (axis_index + 1) % len(axes)
        node.near = self.build_kd_tree(points[:median], axes, next_axis_index)
        node.far = self.build_kd_tree(
            points[median + 1:], axes, next_axis_index)
        if isinstance(node.near, GTYKdNode):
            node.near_index = self.kdtreenodes.index(node.near)
        else:
            node.near_index = -(self.kdtreeleaves.index(node.near) + 1)
        if isinstance(node.far, GTYKdNode):
            node.far_index = self.kdtreenodes.index(node.far)
        else:
            node.far_index = -(self.kdtreeleaves.index(node.far) + 1)
        return node

    def classify_point_to_kd_node(self, point, node):
        distance = point[node.axis] - node.distance
        if distance > 0.00001:
            return 1
        if distance < -0.00001:
            return -1
        return 0

    def classify_polygon_to_kd_node(self, polygon, vertices, node):
        front = 0
        behind = 0
        for vertex in polygon.vertices:
            side = self.classify_point_to_kd_node(vertices[vertex].co, node)
            if side == -1:
                behind = behind + 1
            elif side == 1:
                front = front + 1
        # If there are vertices on both sides of the plane, the polygon is
        # straddling
        if behind != 0 and front != 0:
            return 0
        # If one or more vertices in front of the plane and no vertices behind
        # the plane, the polygon lies in front of the plane
        if front != 0:
            return 1
        # Ditto, the polygon lies behind the plane if no vertices in front of
        # the plane, and one or more vertices behind the plane
        if behind != 0:
            return -1
        # All vertices lie on the plane so the polygon is coplanar with the plane,
        # decide which way to send it based on the polygon normal's mayor axis.
        if polygon.normal[node.axis] < 0.0:
            return -1
        return 1

    def add_brush_to_kd_tree(self, node, polygon, vertices, index):
        if isinstance(node, GTYKdNode):
            side = self.classify_polygon_to_kd_node(polygon, vertices, node)
            if side == -1:
                self.add_brush_to_kd_tree(node.near, polygon, vertices, index)
            elif side == 1:
                self.add_brush_to_kd_tree(node.far, polygon, vertices, index)
            else:
                self.add_brush_to_kd_tree(node.near, polygon, vertices, index)
                self.add_brush_to_kd_tree(node.far, polygon, vertices, index)
        elif isinstance(node, GTYKdLeaf):
            node.brush_indices.append(index)

    def process_collision_mesh(self, mesh_object):
        print("Processing Mesh ", mesh_object.name, " for collision data.")
        mesh = mesh_object.data
        collision_faces = {}
        count = 1
        for polygon in mesh.polygons:
            print("Processing polygon ", count, " out of ", len(mesh.polygons))
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
            collision_faces[polygon] = [sixdop]
            print("6DOP", collision_faces[polygon])

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
                print("Polygon Plane: ", plane)
                collision_faces[polygon].append(self.addplane(plane))
            # else:
            #    print("Polygon plane matches a 6-DOP, discarded")

        # Calculate edge planes, must take neighbouring polygon into account
        # (if any)
        count = 1
        for edge in mesh.edges:
            print("Processing edge ", count, " out of ", len(mesh.edges))
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
                    [normal[0], normal[1], normal[2], distance])
                collision_faces[adjacent_faces[0]].append(plane_index)
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
                            [normal[0], normal[1], normal[2], distance])
                        # Same plane for both faces.
                        collision_faces[adjacent_faces[0]].append(plane_index)
                        collision_faces[adjacent_faces[1]].append(plane_index)
                        # TODO: Refactor this later.
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
                            [normal[0], normal[1], normal[2], distance])
                        collision_faces[adjacent_faces[0]].append(plane_index)
                        collision_faces[adjacent_faces[1]
                                        ].append(-(plane_index + 1))
                        # End TODO: Refactor this later.
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
                            adjacent_faces[0].normal.dot(mesh.vertices[adjacent_faces[0].vertices[0]].co)])
                        collision_faces[adjacent_faces[1]
                                        ].append(-(plane_index + 1))
                        plane_index = self.addplane([
                            adjacent_faces[1].normal[0],
                            adjacent_faces[1].normal[1],
                            adjacent_faces[1].normal[2],
                            adjacent_faces[1].normal.dot(mesh.vertices[adjacent_faces[1].vertices[0]].co)])
                        collision_faces[adjacent_faces[0]
                                        ].append(-(plane_index + 1))
                        break
            else:
                # either edge has no faces attached or has more than 2, either
                # way it is invalid for collision detection.
                print("Invalid Edge")

        # Calculate vertex planes.
        count = 1
        for vertex in mesh.vertices:
            print("Processing vertex ", count, " out of ", len(mesh.vertices))
            count = count + 1
            distance = vertex.normal.dot(vertex.co)
            # TODO: plane must only be added if the vertex is "sharp".
            plane_index = self.addplane([
                vertex.normal[0],
                vertex.normal[1],
                vertex.normal[2],
                distance])
            for polygon in mesh.polygons:
                if vertex in polygon.vertices:
                    collision_faces[polygon].append(plane_index)

        for polygon, face in collision_faces.items():
            indices = list(set(face[1:]))
            self.add_brush_to_kd_tree(
                self.kdtreenodes[0], polygon, mesh.vertices, len(
                    self.brushes))
            # Store 6-DOP,start brush index, brush index count
            self.brushes.append(
                GTYBrush(
                    face[0], len(
                        self.plane_indices), len(indices)))
            # Store indices
            self.plane_indices.extend(indices)

    def process_visual_mesh(self, mesh_object):
        mesh = mesh_object.data
        for material in mesh.materials:
            print(len(material.texture_slots), " texture slots")
            for texture_slot in material.texture_slots:
                if(texture_slot is not None):
                    print(
                        "Texture Slot: ",
                        texture_slot.name,
                        "UV Layer: ",
                        texture_slot.uv_layer);
            if material not in self.materials:
                self.materials[material] = commoncode.TriangleGroup(
                    self.string_table.add(material.name))
                # Position is always required
                self.materials[material].add_attribute(
                    self.string_table.add("position"),
                    3,
                    commoncode.FLOAT,
                    0,
                    commoncode.TriangleGroup.position_callback_tessface)
                # Normal is optional but nice to have (TODO: add some mechanism
                # to disable normal exporting.).
                self.materials[material].add_attribute(
                    self.string_table.add("normal"),
                    3,
                    commoncode.FLOAT,
                    1,
                    commoncode.TriangleGroup.normal_callback_tessface)
                # Get distinct UV Map names
                uvmaps = []
                for texture_slot in material.texture_slots:
                    if(texture_slot is not None and texture_slot.uv_layer != ""):
                        if texture_slot.uv_layer not in uvmaps:
                            uvmaps.append(texture_slot.uv_layer)
                for uvmap in uvmaps:
                    self.materials[material].add_attribute(
                        self.string_table.add(uvmap),
                        2,
                        commoncode.FLOAT,
                        0,
                        commoncode.TriangleGroup.uv_callback_tessface,
                        uvmap)
        if self.materials:
            mesh.calc_tessface()
            for tessface in mesh.tessfaces:
                self.materials[mesh.materials[tessface.material_index]].add_tessface(
                    tessface, mesh_object)
        else:
            print("No materials found, no visual mesh exported.")

    def execute(self, context):
        seconds = time.time()
        bpy.ops.object.mode_set()
        scene = context.scene
        self.filepath = bpy.path.ensure_ext(self.filepath, ".gty")
        header = GTYHeader()

        for object in scene.objects:
            if (object.type == 'MESH'):
                self.process_kd_mesh(object.data)
        self.build_kd_tree(self.kdpoints, [0, 1, 2], 0)

        for object in scene.objects:
            if (object.type == 'MESH'):
                self.process_collision_mesh(object)
                self.process_visual_mesh(object)
                for point in object.bound_box:
                    world_space_point = mathutils.Vector(
                        point) * mathutils.Matrix(object.matrix_world)
                    if world_space_point[0] < header.aabb[0]:
                        header.aabb[0] = world_space_point[0]
                    if world_space_point[0] > header.aabb[3]:
                        header.aabb[3] = world_space_point[0]
                    if world_space_point[1] < header.aabb[1]:
                        header.aabb[1] = world_space_point[1]
                    if world_space_point[1] > header.aabb[4]:
                        header.aabb[4] = world_space_point[1]
                    if world_space_point[2] < header.aabb[2]:
                        header.aabb[2] = world_space_point[2]
                    if world_space_point[2] > header.aabb[5]:
                        header.aabb[5] = world_space_point[2]

        header.aabb[0] = commoncode.snap(header.aabb[0])
        header.aabb[1] = commoncode.snap(header.aabb[1])
        header.aabb[2] = commoncode.snap(header.aabb[2])
        header.aabb[3] = commoncode.snap(header.aabb[3])
        header.aabb[4] = commoncode.snap(header.aabb[4])
        header.aabb[5] = commoncode.snap(header.aabb[5])
        print(
            "AABB is ",
            header.aabb[0],
            ",",
            header.aabb[1],
            ",",
            header.aabb[2],
            " ",
            header.aabb[3],
            ",",
            header.aabb[4],
            ",",
            header.aabb[5])

        header.tile_id = self.string_table.add(scene.name)
        header.collision_plane_count = len(self.planes)
        header.collision_index_count = len(self.plane_indices)
        header.collision_brush_count = len(self.brushes)
        print("Brushes: ", header.collision_brush_count)
        header.kd_node_count = len(self.kdtreenodes)
        header.kd_leaf_count = len(self.kdtreeleaves)

        out = open(self.filepath, "wb")
        out.seek(GTYHeader.header_struct.size)

        # Write planes
        header.collision_plane_offset = out.tell()
        for plane in self.planes:
            out.write(
                struct.pack(
                    '4f',
                    plane[0],
                    plane[1],
                    plane[2],
                    plane[3]))

        # Write plane indices
        # out.write(struct.pack('3I',self.GTY_COLLISION_INDICES,4,len(self.plane_indices)))
        header.collision_index_offset = out.tell()
        for index in self.plane_indices:
            out.write(struct.pack('i', index))

        # Write COLLISION brushes
        header.collision_brush_offset = out.tell()
        for brush in self.brushes:
            brush.write(out)

        # Write KD Nodes
        header.kd_node_offset = out.tell()
        for node in self.kdtreenodes:
            node.write(out)
        # Write KD Leaves
        header.kd_leaf_offset = out.tell()
        # Write indices first
        out.seek(len(self.kdtreeleaves) * GTYKdLeaf.kdleaf_struct.size, 1)
        for leaf in self.kdtreeleaves:
            leaf.write_brush_indices(out)
        # Return to start of leaves
        out.seek(header.kd_leaf_offset)
        # Write leaves
        for leaf in self.kdtreeleaves:
            leaf.write(out)
        # go back to end of file
        out.seek(0, 2)
        #----------------------------------------------------------------------
        header.triangle_group_count = len(self.materials)
        print("header.triangle_group_count = ", header.triangle_group_count)
        header.triangle_group_offset = out.tell()
        # Move to the start of the attribute data position
        out.seek(header.triangle_group_count *
                 commoncode.TriangleGroup.triangle_group_struct.size, 1)

        # Create a list of materials sorted by id to be used for writing.
        # Having the triangle groups sorted by id makes them searchable
        # without the use of a reference table.
        groups = sorted(self.materials.values(), key=lambda group: group.id)
        for group in groups:
            print("Group ID CRC:", group.id)
            # Write Attributes
            group.write_attributes(out)
        # Write Index Buffer
        header.index_buffer_offset = out.tell()
        header.index_buffer_size = 0
        for group in groups:
            header.index_buffer_size = header.index_buffer_size + \
                group.write_index_buffer(out)
        # Write Vertex Buffer
        header.vertex_buffer_offset = out.tell()
        header.vertex_buffer_size = 0
        for group in groups:
            header.vertex_buffer_size = header.vertex_buffer_size + \
                group.write_vertex_buffer(out)
        # Go back to Triangle Groups offset and write their headers
        out.seek(header.triangle_group_offset)
        for group in groups:
            group.write(
                out,
                header.index_buffer_offset,
                header.vertex_buffer_offset)
        # All visual data written go back to end of file
        out.seek(0, 2)
        #----------------------------------------------------------------------
        # Write string table
        header.string_table_offset = self.string_table.write(out)
        # go back to start of file
        out.seek(0)
        # Write Header
        header.write(out)
        # Close file
        out.close()
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
                ".gty")
        else:
            self.filepath = bpy.path.ensure_ext(self.filepath, ".gty")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

def gty_menu_func(self, context):
    self.layout.operator(
        GTYExporter.bl_idname,
        text="AeonGames Level Geometry (.gty)")

def register():
    bpy.utils.register_class(GTYExporter)
    bpy.types.INFO_MT_file_export.append(gty_menu_func)

if __name__ == "__main__":
    register()

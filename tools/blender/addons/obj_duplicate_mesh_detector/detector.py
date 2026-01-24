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
import mathutils
from collections import defaultdict


def get_uv_data(obj):
    """
    Extract UV map data from a mesh object.
    
    Args:
        obj: Blender mesh object
        
    Returns:
        Dictionary with UV layer names as keys and sorted UV coordinate lists as values
    """
    mesh = obj.data
    uv_data = {}
    
    for uv_layer in mesh.uv_layers:
        uvs = []
        for loop_idx, loop in enumerate(mesh.loops):
            uv = uv_layer.data[loop_idx].uv
            uvs.append((loop.vertex_index, uv.x, uv.y))
        uv_data[uv_layer.name] = sorted(uvs)
    
    return uv_data


def compare_uv_data(uv_data1, uv_data2, tolerance):
    """
    Compare UV data from two meshes to determine if they match.
    
    Args:
        uv_data1: First UV data dict
        uv_data2: Second UV data dict
        tolerance: Distance tolerance for matching
        
    Returns:
        True if at least one UV layer matches between the meshes
    """
    # If either has no UV layers, consider them matching (UV not a factor)
    if not uv_data1 or not uv_data2:
        return True
    
    tolerance_sq = tolerance * tolerance
    
    # Check if any UV layer in mesh1 matches any in mesh2
    for name1, uvs1 in uv_data1.items():
        for name2, uvs2 in uv_data2.items():
            if len(uvs1) != len(uvs2):
                continue
            
            match = True
            for (vi1, u1, v1), (vi2, u2, v2) in zip(uvs1, uvs2):
                if vi1 != vi2:
                    match = False
                    break
                du = u1 - u2
                dv = v1 - v2
                dist_sq = du * du + dv * dv
                if dist_sq > tolerance_sq:
                    match = False
                    break
            
            if match:
                return True
    
    return False


def get_point_cloud(obj, use_world_space=False):
    """
    Extract point cloud (vertex positions) from a mesh object.
    
    Args:
        obj: Blender mesh object
        use_world_space: If True, transform vertices to world space
        
    Returns:
        List of vertex coordinates as tuples, sorted for consistent comparison
    """
    mesh = obj.data
    vertices = []
    
    for vertex in mesh.vertices:
        if use_world_space:
            # Transform vertex to world space
            world_co = obj.matrix_world @ vertex.co
            vertices.append((world_co.x, world_co.y, world_co.z))
        else:
            vertices.append((vertex.co.x, vertex.co.y, vertex.co.z))
    
    # Sort vertices to make comparison order-independent
    return sorted(vertices)


def compute_point_cloud_hash(point_cloud, tolerance):
    """
    Compute a hash for quick comparison of point clouds.
    Uses quantized coordinates based on tolerance.
    
    Args:
        point_cloud: Sorted list of vertex coordinates
        tolerance: Distance tolerance for quantization
        
    Returns:
        Tuple that can be used as a hash key
    """
    if tolerance <= 0:
        tolerance = 0.0001
    
    # Quantize coordinates based on tolerance
    quantized = []
    for (x, y, z) in point_cloud:
        qx = round(x / tolerance)
        qy = round(y / tolerance)
        qz = round(z / tolerance)
        quantized.append((qx, qy, qz))
    
    return (len(point_cloud), tuple(quantized))


def compare_point_clouds(cloud1, cloud2, tolerance):
    """
    Compare two point clouds to determine if they are duplicates.
    
    Args:
        cloud1: First point cloud (sorted list of vertex tuples)
        cloud2: Second point cloud (sorted list of vertex tuples)
        tolerance: Distance tolerance for matching
        
    Returns:
        True if the point clouds are considered duplicates
    """
    # Quick check: must have same number of vertices
    if len(cloud1) != len(cloud2):
        return False
    
    # Compare each vertex pair
    tolerance_sq = tolerance * tolerance
    for (x1, y1, z1), (x2, y2, z2) in zip(cloud1, cloud2):
        dx = x1 - x2
        dy = y1 - y2
        dz = z1 - z2
        dist_sq = dx * dx + dy * dy + dz * dz
        if dist_sq > tolerance_sq:
            return False
    
    return True


def find_duplicate_meshes(objects, tolerance, use_world_space, compare_uvs=False):
    """
    Find groups of duplicate meshes among the given objects.
    
    Args:
        objects: List of mesh objects to compare
        tolerance: Distance tolerance for matching
        use_world_space: Whether to compare in world space
        compare_uvs: Whether to also compare UV maps
        
    Returns:
        List of lists, where each inner list contains names of duplicate meshes
    """
    # Extract point clouds for all meshes
    mesh_data = []
    for obj in objects:
        if obj.type == 'MESH' and obj.data:
            point_cloud = get_point_cloud(obj, use_world_space)
            cloud_hash = compute_point_cloud_hash(point_cloud, tolerance)
            data = {
                'object': obj,
                'name': obj.name,
                'point_cloud': point_cloud,
                'hash': cloud_hash
            }
            if compare_uvs:
                data['uv_data'] = get_uv_data(obj)
            mesh_data.append(data)
    
    # Group by hash for quick filtering
    hash_groups = defaultdict(list)
    for data in mesh_data:
        hash_groups[data['hash']].append(data)
    
    # Find actual duplicates within each hash group
    duplicate_groups = []
    processed = set()
    
    for hash_key, group in hash_groups.items():
        if len(group) < 2:
            continue
        
        # Within each hash group, verify with detailed comparison
        for i, data1 in enumerate(group):
            if data1['name'] in processed:
                continue
            
            current_group = [data1['name']]
            
            for j, data2 in enumerate(group[i + 1:], start=i + 1):
                if data2['name'] in processed:
                    continue
                
                is_duplicate = compare_point_clouds(data1['point_cloud'], data2['point_cloud'], tolerance)
                
                # If UV comparison is enabled, also check UV data
                if is_duplicate and compare_uvs:
                    is_duplicate = compare_uv_data(data1['uv_data'], data2['uv_data'], tolerance)
                
                if is_duplicate:
                    current_group.append(data2['name'])
                    processed.add(data2['name'])
            
            if len(current_group) > 1:
                duplicate_groups.append(current_group)
                processed.add(data1['name'])
    
    return duplicate_groups


def convert_to_linked(duplicate_groups, scene):
    """
    Convert duplicate mesh groups to linked copies, merging UV maps.
    
    For each group of duplicates:
    1. Keep the first object's mesh as the base
    2. Copy all distinct UV maps from other objects into the base mesh
    3. Link all other objects to use the base mesh
    4. Set each object to use the UV map that corresponds to its original
    
    Args:
        duplicate_groups: List of lists containing object names
        scene: Blender scene
        
    Returns:
        Number of objects converted
    """
    converted_count = 0
    
    for group in duplicate_groups:
        if len(group) < 2:
            continue
        
        # Get the base object (first in group)
        base_obj = scene.objects.get(group[0])
        if not base_obj or base_obj.type != 'MESH':
            continue
        
        base_mesh = base_obj.data
        
        # Track UV map assignments for each object
        # Key: object name, Value: UV layer name to use
        uv_assignments = {}
        
        # The base object uses its active UV map (or first if none active)
        if base_mesh.uv_layers:
            active_uv = base_mesh.uv_layers.active
            uv_assignments[base_obj.name] = active_uv.name if active_uv else base_mesh.uv_layers[0].name
        
        # Process each duplicate
        for obj_name in group[1:]:
            dup_obj = scene.objects.get(obj_name)
            if not dup_obj or dup_obj.type != 'MESH':
                continue
            
            dup_mesh = dup_obj.data
            
            # Find or create UV map for this duplicate
            if dup_mesh.uv_layers:
                # Get the active UV layer from the duplicate
                active_uv = dup_mesh.uv_layers.active
                source_uv_name = active_uv.name if active_uv else dup_mesh.uv_layers[0].name
                source_uv = dup_mesh.uv_layers.get(source_uv_name)
                
                if source_uv:
                    # Check if an equivalent UV map already exists in base mesh
                    matching_uv_name = find_matching_uv_layer(base_mesh, source_uv, dup_mesh)
                    
                    if matching_uv_name:
                        # Use existing matching UV layer
                        uv_assignments[obj_name] = matching_uv_name
                    else:
                        # Create a new UV layer in base mesh with unique name
                        new_uv_name = get_unique_uv_name(base_mesh, f"{obj_name}_UV")
                        new_uv = base_mesh.uv_layers.new(name=new_uv_name)
                        
                        # Copy UV data
                        copy_uv_data(source_uv, new_uv, dup_mesh, base_mesh)
                        
                        uv_assignments[obj_name] = new_uv_name
            
            # Store original mesh reference before replacing
            old_mesh = dup_mesh
            
            # Link to base mesh
            dup_obj.data = base_mesh
            
            # Remove old mesh if no longer used
            if old_mesh.users == 0:
                bpy.data.meshes.remove(old_mesh)
            
            converted_count += 1
        
        # Set active UV layer for each object based on assignments
        for obj_name, uv_name in uv_assignments.items():
            obj = scene.objects.get(obj_name)
            if obj and obj.data == base_mesh:
                uv_layer = base_mesh.uv_layers.get(uv_name)
                if uv_layer:
                    base_mesh.uv_layers.active = uv_layer
                    # Store the UV assignment as a custom property on the object
                    obj["duplicate_mesh_uv"] = uv_name
    
    return converted_count


def find_matching_uv_layer(base_mesh, source_uv, source_mesh, tolerance=0.0001):
    """
    Find a UV layer in base_mesh that matches the source UV layer.
    
    Args:
        base_mesh: Mesh to search in
        source_uv: Source UV layer to match
        source_mesh: Mesh containing the source UV layer
        tolerance: Matching tolerance
        
    Returns:
        Name of matching UV layer or None
    """
    tolerance_sq = tolerance * tolerance
    
    for uv_layer in base_mesh.uv_layers:
        if len(uv_layer.data) != len(source_uv.data):
            continue
        
        match = True
        for i in range(len(uv_layer.data)):
            uv1 = uv_layer.data[i].uv
            uv2 = source_uv.data[i].uv
            du = uv1.x - uv2.x
            dv = uv1.y - uv2.y
            if du * du + dv * dv > tolerance_sq:
                match = False
                break
        
        if match:
            return uv_layer.name
    
    return None


def get_unique_uv_name(mesh, base_name):
    """
    Generate a unique UV layer name for the mesh.
    
    Args:
        mesh: Blender mesh
        base_name: Desired base name
        
    Returns:
        Unique name string
    """
    existing_names = {uv.name for uv in mesh.uv_layers}
    
    if base_name not in existing_names:
        return base_name
    
    counter = 1
    while f"{base_name}.{counter:03d}" in existing_names:
        counter += 1
    
    return f"{base_name}.{counter:03d}"


def copy_uv_data(source_uv, dest_uv, source_mesh, dest_mesh):
    """
    Copy UV data from source to destination UV layer.
    Assumes meshes have the same topology.
    
    Args:
        source_uv: Source UV layer
        dest_uv: Destination UV layer
        source_mesh: Source mesh
        dest_mesh: Destination mesh
    """
    # Direct copy assuming same loop order
    for i in range(min(len(source_uv.data), len(dest_uv.data))):
        dest_uv.data[i].uv = source_uv.data[i].uv.copy()


class DUPLICATEMESH_OT_detect(bpy.types.Operator):
    """Detect duplicate meshes based on point cloud comparison"""
    bl_idname = "object.detect_duplicate_meshes"
    bl_label = "Detect Duplicates"
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        return context.mode == 'OBJECT'

    def execute(self, context):
        scene = context.scene
        tolerance = scene.duplicate_mesh_tolerance
        use_world_space = scene.duplicate_mesh_use_world_space
        selected_only = scene.duplicate_mesh_selected_only
        compare_uvs = scene.duplicate_mesh_compare_uvs
        
        # Get objects to compare
        if selected_only:
            objects = [obj for obj in context.selected_objects if obj.type == 'MESH']
        else:
            objects = [obj for obj in scene.objects if obj.type == 'MESH']
        
        if len(objects) < 2:
            self.report({'WARNING'}, "Need at least 2 mesh objects to compare")
            scene.duplicate_mesh_results = ""
            return {'CANCELLED'}
        
        # Find duplicates
        duplicate_groups = find_duplicate_meshes(objects, tolerance, use_world_space, compare_uvs)
        
        if duplicate_groups:
            # Format results for display
            result_strings = []
            for i, group in enumerate(duplicate_groups, start=1):
                result_strings.append(f"Group {i}: {', '.join(group)}")
            
            scene.duplicate_mesh_results = "|".join(result_strings)
            
            # Select duplicate objects for visibility
            bpy.ops.object.select_all(action='DESELECT')
            for group in duplicate_groups:
                for name in group:
                    if name in scene.objects:
                        scene.objects[name].select_set(True)
            
            total_duplicates = sum(len(g) for g in duplicate_groups)
            self.report({'INFO'}, f"Found {len(duplicate_groups)} groups with {total_duplicates} duplicate meshes")
        else:
            scene.duplicate_mesh_results = ""
            self.report({'INFO'}, "No duplicate meshes found")
        
        return {'FINISHED'}


class DUPLICATEMESH_OT_convert_linked(bpy.types.Operator):
    """Convert detected duplicate meshes to linked copies, merging UV maps"""
    bl_idname = "object.convert_duplicates_to_linked"
    bl_label = "Convert to Linked"
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        # Only enable if there are results
        return context.mode == 'OBJECT' and context.scene.duplicate_mesh_results

    def execute(self, context):
        scene = context.scene
        
        # Parse the stored results to get duplicate groups
        if not scene.duplicate_mesh_results:
            self.report({'WARNING'}, "No duplicate groups found. Run detection first.")
            return {'CANCELLED'}
        
        duplicate_groups = []
        for group_str in scene.duplicate_mesh_results.split("|"):
            if group_str:
                # Parse "Group N: obj1, obj2, obj3" format
                if ": " in group_str:
                    names_str = group_str.split(": ", 1)[1]
                    names = [n.strip() for n in names_str.split(", ")]
                    if len(names) > 1:
                        duplicate_groups.append(names)
        
        if not duplicate_groups:
            self.report({'WARNING'}, "No valid duplicate groups to convert")
            return {'CANCELLED'}
        
        # Convert to linked
        converted = convert_to_linked(duplicate_groups, scene)
        
        if converted > 0:
            self.report({'INFO'}, f"Converted {converted} objects to linked copies with merged UV maps")
            # Clear results since the duplicates are now linked
            scene.duplicate_mesh_results = ""
        else:
            self.report({'WARNING'}, "No objects were converted")
        
        return {'FINISHED'}

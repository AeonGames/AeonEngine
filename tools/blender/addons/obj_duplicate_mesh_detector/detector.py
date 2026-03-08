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
import numpy as np
import os
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
import math

# Determine a sensible thread count once at import time.
_CPU_COUNT = os.cpu_count() or 4


def compute_centroid(point_cloud):
    """
    Compute the centroid (center of mass) of a point cloud.
    
    Args:
        point_cloud: List of (x, y, z) tuples
        
    Returns:
        Tuple (cx, cy, cz) representing the centroid
    """
    if not point_cloud:
        return (0.0, 0.0, 0.0)
    
    n = len(point_cloud)
    cx = sum(p[0] for p in point_cloud) / n
    cy = sum(p[1] for p in point_cloud) / n
    cz = sum(p[2] for p in point_cloud) / n
    
    return (cx, cy, cz)


def compute_bounding_box(point_cloud):
    """
    Compute the axis-aligned bounding box of a point cloud.

    Returns:
        Tuple ((min_x, min_y, min_z), (max_x, max_y, max_z))
    """
    if not point_cloud:
        return ((0.0, 0.0, 0.0), (0.0, 0.0, 0.0))
    xs = [p[0] for p in point_cloud]
    ys = [p[1] for p in point_cloud]
    zs = [p[2] for p in point_cloud]
    return ((min(xs), min(ys), min(zs)), (max(xs), max(ys), max(zs)))


def bounding_box_extents(bb_min, bb_max):
    """
    Return sorted extents (dx, dy, dz) of a bounding box.
    Sorting makes the comparison rotation-invariant for axis-aligned rotations.
    """
    return tuple(sorted((
        bb_max[0] - bb_min[0],
        bb_max[1] - bb_min[1],
        bb_max[2] - bb_min[2],
    )))


def estimate_umeyama_transform(cloud1, cloud2, allow_scale=False, allow_non_uniform_scale=False):
    """
        Estimate transform from cloud1 to cloud2 using Umeyama/Kabsch SVD.

    Notes:
      - Requires one-to-one correspondence by index.
      - Reflection is disallowed (proper rotation only).
            - If allow_scale is True, estimates a uniform scale as well.

    Returns:
        (R, t, scale, rmse) where
            R: 3x3 rotation matrix
            t: 3D translation vector
            scale: uniform scale factor (float) or per-axis scale (tuple)
            rmse: per-point correspondence RMSE after transform
        or (None, None, 1.0, inf) on failure.
    """
    if len(cloud1) != len(cloud2) or len(cloud1) < 3:
        return None, None, 1.0, float('inf')

    x = np.asarray(cloud1, dtype=np.float64)
    y = np.asarray(cloud2, dtype=np.float64)
    n = x.shape[0]

    mx = x.mean(axis=0)
    my = y.mean(axis=0)
    xc = x - mx
    yc = y - my

    cov = (yc.T @ xc) / float(n)
    u, singular_values, vt = np.linalg.svd(cov)

    s = np.eye(3)
    if np.linalg.det(u) * np.linalg.det(vt) < 0:
        s[2, 2] = -1.0

    r = u @ s @ vt

    scale = 1.0
    if allow_non_uniform_scale:
        # Estimate anisotropic scale in rotated frame: yc ~= diag(s) * (r * xc)
        xr = (r @ xc.T).T
        num = np.sum(yc * xr, axis=0)
        den = np.sum(xr * xr, axis=0)
        scale_vec = np.ones(3, dtype=np.float64)
        valid = den > 1e-15
        scale_vec[valid] = num[valid] / den[valid]
        scale = tuple(float(v) for v in scale_vec)
    elif allow_scale:
        var_x = float(np.sum(xc * xc) / n)
        if var_x > 0:
            # Umeyama uniform scale: tr(D * S) / var(X)
            scale = float(np.sum(np.diag(s) * singular_values) / var_x)

    if isinstance(scale, tuple):
        scale_vec = np.asarray(scale, dtype=np.float64)
        t = my - (scale_vec * (r @ mx))
    else:
        t = my - (scale * (r @ mx))

    if isinstance(scale, tuple):
        scale_vec = np.asarray(scale, dtype=np.float64)
        x_aligned = (scale_vec * (r @ x.T).T) + t
    else:
        x_aligned = (scale * (r @ x.T)).T + t
    rmse = float(np.sqrt(np.mean(np.sum((x_aligned - y) ** 2, axis=1))))

    return r, t, scale, rmse


def apply_transform(point_cloud, rotation, translation, scale=1.0):
    """Apply a similarity transform (uniform scale + rotation + translation) to a point cloud."""
    arr = np.asarray(point_cloud, dtype=np.float64)
    rotated = (rotation @ arr.T).T
    if isinstance(scale, tuple):
        scale_vec = np.asarray(scale, dtype=np.float64)
        aligned = (scale_vec * rotated) + translation
    else:
        aligned = (scale * rotated) + translation
    return [tuple(p) for p in aligned]


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
    # Fast bulk extraction via foreach_get
    n = len(mesh.vertices)
    coords = np.empty(n * 3, dtype=np.float64)
    mesh.vertices.foreach_get('co', coords)
    coords = coords.reshape((n, 3))

    if use_world_space:
        mat = np.array(obj.matrix_world, dtype=np.float64)  # 4x4
        ones = np.ones((n, 1), dtype=np.float64)
        coords_h = np.hstack([coords, ones])  # Nx4
        coords = (mat @ coords_h.T).T[:, :3]

    # Preserve original vertex order so correspondence-based methods
    # (e.g. Umeyama) can use index-to-index mapping.
    return [tuple(row) for row in coords]


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

    quantized.sort()  # keep hash invariant to vertex ordering

    return (len(point_cloud), tuple(quantized))


def compare_point_clouds_internal(cloud1, cloud2, tolerance, match_threshold=100.0, verbose=True):
    """
    Internal function to compare two point clouds without rotation alignment.
    Uses spatial matching - finds nearest neighbor for each vertex.
    
    Args:
        cloud1: First point cloud (list of vertex tuples)
        cloud2: Second point cloud (list of vertex tuples)
        tolerance: Distance tolerance for matching
        match_threshold: Minimum percentage of vertices that must match (default 100%)
        verbose: Whether to print detailed logging
        
    Returns:
        Tuple of (is_duplicate, match_percentage)
    """
    # Quick check: must have same number of vertices
    if len(cloud1) != len(cloud2):
        return False, 0.0
    
    total_vertices = len(cloud1)
    if total_vertices == 0:
        return True, 100.0

    tolerance_sq = tolerance * tolerance
    matching_vertices = 0

    # --- Compute the maximum number of misses we can tolerate ----------
    max_misses = int(total_vertices * (1.0 - match_threshold / 100.0))
    misses = 0

    # --- Grid construction (tolerance-based, no brute-force fallback) --
    # Use tolerance as the cell size so that a match is always in a
    # neighbouring cell (±1 in each axis).  This guarantees coverage and
    # avoids the costly brute-force path that existed before.
    grid_size = max(tolerance, 1e-12)  # avoid zero
    inv_grid = 1.0 / grid_size
    
    cloud2_grid = defaultdict(list)
    for idx, (x, y, z) in enumerate(cloud2):
        gk = (int(x * inv_grid), int(y * inv_grid), int(z * inv_grid))
        cloud2_grid[gk].append((x, y, z, idx))
    
    used_cloud2_indices = set()
    
    for x1, y1, z1 in cloud1:
        gk = (int(x1 * inv_grid), int(y1 * inv_grid), int(z1 * inv_grid))
        
        min_dist_sq = float('inf')
        best_match_idx = -1
        
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                for dz in (-1, 0, 1):
                    for x2, y2, z2, idx2 in cloud2_grid.get(
                            (gk[0] + dx, gk[1] + dy, gk[2] + dz), ()):
                        if idx2 in used_cloud2_indices:
                            continue
                        distance_sq = (x1 - x2) ** 2 + (y1 - y2) ** 2 + (z1 - z2) ** 2
                        if distance_sq < min_dist_sq:
                            min_dist_sq = distance_sq
                            best_match_idx = idx2
        
        if min_dist_sq <= tolerance_sq and best_match_idx >= 0:
            matching_vertices += 1
            used_cloud2_indices.add(best_match_idx)
        else:
            misses += 1
            # ---- Early termination: too many misses already ----
            if misses > max_misses:
                match_percentage = (matching_vertices / total_vertices) * 100.0
                if verbose:
                    print(f"  [EARLY OUT] {misses} misses > {max_misses} allowed, ~{match_percentage:.1f}%")
                return False, match_percentage
    
    match_percentage = (matching_vertices / total_vertices) * 100.0
    is_duplicate = match_percentage >= match_threshold
    
    if verbose:
        print(f"  [RESULT] {matching_vertices}/{total_vertices} matched ({match_percentage:.2f}%)")
    
    return is_duplicate, match_percentage


def compare_point_clouds(cloud1, cloud2, tolerance, match_threshold=100.0, allow_scale=False, allow_non_uniform_scale=False):
    """
    Compare two point clouds to determine if they are duplicates
    using Umeyama alignment (rigid or similarity).
    
    Args:
        cloud1: First point cloud (list of vertex tuples)
        cloud2: Second point cloud (list of vertex tuples)
        tolerance: Distance tolerance for matching
        match_threshold: Minimum percentage of vertices that must match (default 100%)
        allow_scale: Whether to estimate uniform scale in Umeyama
        allow_non_uniform_scale: Whether to estimate per-axis scale
        
    Returns:
        Tuple of (is_duplicate, match_percentage, estimated_scale)
    """
    if len(cloud1) != len(cloud2):
        return False, 0.0, 1.0

    if not cloud1:
        return True, 100.0, 1.0

    rotation, translation, scale, rmse = estimate_umeyama_transform(
        cloud1, cloud2,
        allow_scale=allow_scale,
        allow_non_uniform_scale=allow_non_uniform_scale,
    )
    if rotation is None:
        return False, 0.0, 1.0

    aligned_cloud = apply_transform(cloud1, rotation, translation, scale=scale)
    is_duplicate, match_percentage = compare_point_clouds_internal(
        aligned_cloud, cloud2, tolerance, match_threshold, verbose=True
    )

    if isinstance(scale, tuple):
        scale_str = f"({scale[0]:.6f}, {scale[1]:.6f}, {scale[2]:.6f})"
    else:
        scale_str = f"{scale:.6f}"

    print(f"  [UMEYAMA] scale={scale_str}, RMSE={rmse:.6f}, match={match_percentage:.2f}%")
    return is_duplicate, match_percentage, scale


def compare_pair(data1, data2, tolerance, compare_uvs, match_threshold=100.0, allow_scale=False, allow_non_uniform_scale=False):
    """
    Compare a pair of mesh data objects.
    
    Args:
        data1: First mesh data dict
        data2: Second mesh data dict
        tolerance: Distance tolerance for matching
        compare_uvs: Whether to compare UV maps
        match_threshold: Minimum percentage of vertices that must match
        
    Returns:
        Tuple of (name1, name2, is_duplicate, match_percentage, estimated_scale)
    """
    # Skip comparison if objects already share the same mesh data (already linked)
    if data1['mesh_data'] == data2['mesh_data']:
        return (data1['name'], data2['name'], False, 0.0, 1.0)

    # ---- EARLY OUT: hash-based fast rejection ----
    if data1['hash'] != data2['hash']:
        # Hashes differ – only skip the full comparison when rotations
        # would not be attempted (the hash is position-dependent).
        # Even with rotations we can still reject based on bounding-box extents
        # (rotation-invariant).
        pass  # fall through to bounding-box check

    # ---- EARLY OUT: bounding-box extents ----
    if not allow_scale and not allow_non_uniform_scale:
        ext1 = data1['bb_extents']
        ext2 = data2['bb_extents']
        # Sorted extents should be very close for duplicate meshes, even under
        # axis-aligned rotation.
        max_ext = max(ext1[2], ext2[2], 1e-12)
        for a, b in zip(ext1, ext2):
            if abs(a - b) > max(tolerance, max_ext * 0.01):
                return (data1['name'], data2['name'], False, 0.0, 1.0)
                

    # ---- EARLY OUT: face / edge count ----
    if data1['face_count'] != data2['face_count']:
        return (data1['name'], data2['name'], False, 0.0, 1.0)
    if data1['edge_count'] != data2['edge_count']:
        return (data1['name'], data2['name'], False, 0.0, 1.0)

    # ---- Full point-cloud comparison ----
    print(f"Comparing '{data1['name']}' vs '{data2['name']}' ({data1['vertex_count']} verts)")
    
    is_duplicate, match_percentage, estimated_scale = compare_point_clouds(
        data1['point_cloud'], data2['point_cloud'], tolerance, match_threshold,
        allow_scale=allow_scale,
        allow_non_uniform_scale=allow_non_uniform_scale,
    )
    
    # If UV comparison is enabled, also check UV data
    if is_duplicate and compare_uvs:
        is_duplicate = compare_uv_data(data1['uv_data'], data2['uv_data'], tolerance)
    
    return (data1['name'], data2['name'], is_duplicate, match_percentage, estimated_scale)


def find_duplicate_meshes(objects, tolerance, use_world_space, compare_uvs=False, match_threshold=100.0,
                          allow_scale=False, allow_non_uniform_scale=False):
    """
    Find groups of duplicate meshes among the given objects.
    
    Args:
        objects: List of mesh objects to compare
        tolerance: Distance tolerance for matching
        use_world_space: Whether to compare in world space
        compare_uvs: Whether to also compare UV maps
        match_threshold: Minimum percentage of vertices that must match
        
    Returns:
                Tuple of:
                    - List of lists, where each inner list contains names of duplicate meshes
                    - List of per-pair scale strings for accepted duplicate matches
    """
    # Extract point clouds for all meshes
    print(f"\n=== Starting duplicate detection for {len(objects)} objects ===")
    mesh_data = []
    for obj in objects:
        if obj.type == 'MESH' and obj.data:
            point_cloud = get_point_cloud(obj, use_world_space)
            cloud_hash = compute_point_cloud_hash(point_cloud, tolerance)
            bb_min, bb_max = compute_bounding_box(point_cloud)
            bb_ext = bounding_box_extents(bb_min, bb_max)
            data = {
                'object': obj,
                'name': obj.name,
                'mesh_data': obj.data,
                'point_cloud': point_cloud,
                'hash': cloud_hash,
                'vertex_count': len(point_cloud),
                'bb_extents': bb_ext,
                'face_count': len(obj.data.polygons),
                'edge_count': len(obj.data.edges),
            }
            if compare_uvs:
                data['uv_data'] = get_uv_data(obj)
            mesh_data.append(data)
    
    # Group by vertex count first (meshes must have same vertex count to be duplicates)
    vertex_count_groups = defaultdict(list)
    for data in mesh_data:
        vertex_count_groups[data['vertex_count']].append(data)
    
    print(f"\n=== Vertex count grouping results: {len(vertex_count_groups)} unique vertex counts ===")
    for vertex_count, group in vertex_count_groups.items():
        obj_names = [item['name'] for item in group]
        print(f"Vertex count {vertex_count} ({len(group)} objects): {', '.join(obj_names)}")
    
    # Find actual duplicates by comparing all objects with the same vertex count.
    # Build all comparison jobs first, then execute in one global thread pool for
    # better utilization across groups.
    duplicate_groups = []
    scale_logs = []

    print(f"\n=== Starting detailed comparison with multithreading ===")

    comparison_jobs = []  # (vertex_count, data1, data2)
    for vertex_count, group in vertex_count_groups.items():
        if len(group) < 2:
            print(f"Vertex count {vertex_count}: Only 1 object, skipping")
            continue

        pair_count = (len(group) * (len(group) - 1)) // 2
        print(f"\nVertex count {vertex_count}: {pair_count} pairs")
        for i, data1 in enumerate(group):
            for j in range(i + 1, len(group)):
                comparison_jobs.append((vertex_count, data1, group[j]))

    if not comparison_jobs:
        return duplicate_groups, scale_logs

    print(f"\nSubmitting {len(comparison_jobs)} total comparisons to {_CPU_COUNT} threads")

    matches_by_vertex_count = defaultdict(list)
    with ThreadPoolExecutor(max_workers=min(len(comparison_jobs), _CPU_COUNT)) as executor:
        futures = {
            executor.submit(
                compare_pair,
                data1,
                data2,
                tolerance,
                compare_uvs,
                match_threshold,
                allow_scale,
                allow_non_uniform_scale,
            ): vertex_count
            for vertex_count, data1, data2 in comparison_jobs
        }

        for future in as_completed(futures):
            vertex_count = futures[future]
            name1, name2, is_duplicate, _match_percentage, estimated_scale = future.result()
            if is_duplicate:
                matches_by_vertex_count[vertex_count].append((name1, name2))
                if isinstance(estimated_scale, tuple):
                    scale_text = f"({estimated_scale[0]:.6f}, {estimated_scale[1]:.6f}, {estimated_scale[2]:.6f})"
                else:
                    scale_text = f"{estimated_scale:.6f}"
                scale_logs.append(f"{name1} <-> {name2}: scale={scale_text}")

    # Build connected components per vertex-count bucket.
    for vertex_count, matches in matches_by_vertex_count.items():
        graph = defaultdict(set)
        for name1, name2 in matches:
            graph[name1].add(name2)
            graph[name2].add(name1)

        visited = set()
        for node in graph:
            if node in visited:
                continue

            component = []
            queue = [node]
            visited.add(node)

            while queue:
                current = queue.pop(0)
                component.append(current)

                for neighbor in graph[current]:
                    if neighbor not in visited:
                        visited.add(neighbor)
                        queue.append(neighbor)

            if len(component) > 1:
                duplicate_groups.append(component)

    return duplicate_groups, scale_logs


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
        match_threshold = scene.duplicate_mesh_match_threshold
        allow_scale = scene.duplicate_mesh_allow_scale
        allow_non_uniform_scale = scene.duplicate_mesh_allow_non_uniform_scale
        
        # Get objects to compare
        if selected_only:
            # Use context.selected_objects but filter to ensure they're in the current view layer
            objects = [obj for obj in context.selected_objects if obj.type == 'MESH' and obj.name in context.view_layer.objects]
        else:
            objects = [obj for obj in scene.objects if obj.type == 'MESH']
        
        if len(objects) < 2:
            self.report({'WARNING'}, "Need at least 2 mesh objects to compare")
            scene.duplicate_mesh_results = ""
            scene.duplicate_mesh_scale_results = ""
            return {'CANCELLED'}
        
        # Find duplicates
        duplicate_groups, scale_logs = find_duplicate_meshes(
            objects, tolerance, use_world_space, compare_uvs, match_threshold,
            allow_scale, allow_non_uniform_scale
        )
        
        if duplicate_groups:
            # Format results for display
            result_strings = []
            for i, group in enumerate(duplicate_groups, start=1):
                result_strings.append(f"Group {i}: {', '.join(group)}")
            
            scene.duplicate_mesh_results = "|".join(result_strings)
            scene.duplicate_mesh_scale_results = "|".join(scale_logs)
            
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
            scene.duplicate_mesh_scale_results = ""
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
            scene.duplicate_mesh_scale_results = ""
        else:
            self.report({'WARNING'}, "No objects were converted")
        
        return {'FINISHED'}

# Copyright (C) 2017,2026 Rodrigo Jose Hernandez Cordoba
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

"""
PSK/PSKX and PSA Importer for Blender 4.2+

Supports:
- PSK: Legacy format (UE1, UE2, UE3/UDK)
- PSKX: Extended format (UE4, UE5) with:
  - 32-bit vertex indices
  - Multiple UV channels (EXTRAUV0, EXTRAUV1, etc.)
  - Vertex normals (VTXNORMS)
  - Vertex colors (VERTEXCOLOR)
  - Morph targets/Shape keys (MRPHINFO, MRPHDATA)
- PSA: Animation format (all UE versions)
"""

import bpy
import bmesh
import mathutils
import itertools
import os
from multiprocessing.dummy import Pool as ThreadPool

from bpy.props import StringProperty, BoolProperty, FloatProperty
from struct import unpack, calcsize

# Coordinate system conversion matrix (Unreal to Blender)
right_hand_matrix = mathutils.Matrix(((-1,0,0,0),(0,-1,0,0),(0,0,-1,0),(0,0,0,1)))


# =============================================================================
# Data Classes
# =============================================================================

class PskData:
    """Container for PSK file data"""
    def __init__(self):
        self.points = []           # List of (x, y, z) vertices
        self.wedges = []           # List of Wedge objects
        self.faces = []            # List of Face objects
        self.materials = []        # List of material names
        self.bones = []            # List of RawEditBone objects
        self.weights = []          # List of (point_idx, bone_idx, weight)
        self.extra_uvs = []        # List of UV channel lists (PSKX)
        self.vertex_colors = []    # List of (r, g, b, a) tuples (PSKX)
        self.vertex_normals = []   # List of (x, y, z) normals (PSKX)
        self.morph_infos = []      # List of (name, vertex_count) (PSKX)
        self.morph_data = []       # List of morph vertex data (PSKX)
        self.is_pskx = False       # True if extended format features detected


class Wedge:
    """Vertex with UV coordinates"""
    def __init__(self, point_index, u, v, material_index=0):
        self.point_index = point_index
        self.u = u
        self.v = v
        self.material_index = material_index


class Face:
    """Triangle face"""
    def __init__(self, wedge_indices, material_index, aux_material_index, smoothing_groups):
        self.wedge_indices = wedge_indices  # (w0, w1, w2)
        self.material_index = material_index
        self.aux_material_index = aux_material_index
        self.smoothing_groups = smoothing_groups


class RawEditBone:
    """Bone data with computed matrix"""
    def __init__(self, name, parent, rotation, translation):
        self.name = name
        self.parent = parent
        self.rotation = rotation
        self.translation = translation
        
        rotation_matrix = self.rotation.to_matrix().to_4x4()
        translation_matrix = mathutils.Matrix.Translation(self.translation)
        self.matrix = right_hand_matrix @ (rotation_matrix @ translation_matrix) @ right_hand_matrix
        if self.parent is not None:
            self.matrix = self.parent.matrix @ self.matrix.inverted()


class RawAction:
    """Animation sequence data"""
    def __init__(self, data_tuple):
        (
            self.Name,
            self.Group,
            self.TotalBones,
            self.RootInclude,
            self.KeyCompressionStyle,
            self.KeyQuotum,
            self.KeyReduction,
            self.TrackTime,
            self.AnimRate,
            self.StartBone,
            self.FirstRawFrame,
            self.NumRawFrames
        ) = data_tuple


class RawKey:
    """Animation keyframe data"""
    def __init__(self, position, orientation, time):
        self.Position = mathutils.Vector(position)
        # Reorder quaternion from XYZW to WXYZ
        self.Orientation = mathutils.Quaternion((orientation[3], orientation[0], orientation[1], orientation[2]))
        self.Time = time


# =============================================================================
# Helper Functions
# =============================================================================

def bytes_to_string(data):
    """Convert bytes to string, handling null termination"""
    if isinstance(data, bytes):
        return data.decode('utf-8', errors='replace').rstrip('\0')
    return data


def read_section_header(f):
    """Read a PSK/PSA section header (20 bytes name + 3 ints)"""
    data = f.read(32)
    if len(data) < 32:
        return None, 0, 0, 0
    name, type_flags, data_size, data_count = unpack('20s3i', data)
    return name.rstrip(b'\0'), type_flags, data_size, data_count


# =============================================================================
# PSK/PSKX Reader
# =============================================================================

def read_psk(filepath):
    """
    Read PSK or PSKX file.
    
    Supports both legacy PSK (UE1-3) and extended PSKX (UE4-5) formats.
    """
    psk = PskData()
    
    with open(filepath, 'rb') as f:
        while True:
            section_name, type_flags, data_size, data_count = read_section_header(f)
            if section_name is None:
                break
            
            if section_name == b'ACTRHEAD':
                # File header, no data
                pass
            
            elif section_name == b'PNTS0000':
                # Vertices: 3 floats (12 bytes each)
                for _ in range(data_count):
                    x, y, z = unpack('3f', f.read(12))
                    psk.points.append((x, y, z))
            
            elif section_name == b'VTXW0000':
                # Wedges (vertices with UVs)
                # Can be 16-byte (legacy) or 16-byte with 32-bit material index
                if data_size == 16:
                    # Standard format: point_index(I), u(f), v(f), material(B), reserved(b), padding(h)
                    for _ in range(data_count):
                        point_idx, u, v, mat_idx, reserved, padding = unpack('IffBbh', f.read(16))
                        # Handle potential 32-bit point index stored in padding
                        if len(psk.points) <= 65536:
                            point_idx = point_idx & 0xFFFF
                        psk.wedges.append(Wedge(point_idx, u, v, mat_idx))
                elif data_size == 12:
                    # Legacy format: point_index(H), padding(H), u(f), v(f)
                    for _ in range(data_count):
                        point_idx, padding, u, v = unpack('HHff', f.read(12))
                        psk.wedges.append(Wedge(point_idx, u, v, 0))
                else:
                    # Unknown wedge format, try to read based on size
                    f.seek(data_size * data_count, os.SEEK_CUR)
            
            elif section_name == b'FACE0000':
                # Faces with 16-bit wedge indices
                # Format: wedge_indices(3H), material(B), aux_material(B), smoothing_groups(i)
                for _ in range(data_count):
                    w0, w1, w2, mat_idx, aux_mat, smooth = unpack('3HBBi', f.read(12))
                    psk.faces.append(Face((w0, w1, w2), mat_idx, aux_mat, smooth))
            
            elif section_name == b'FACE3200':
                # PSKX: Faces with 32-bit wedge indices
                psk.is_pskx = True
                for _ in range(data_count):
                    w0, w1, w2, mat_idx, aux_mat, smooth = unpack('3IBBi', f.read(18))
                    psk.faces.append(Face((w0, w1, w2), mat_idx, aux_mat, smooth))
            
            elif section_name == b'MATT0000':
                # Materials: 64-byte name + 6 ints
                for _ in range(data_count):
                    mat_name = f.read(64)
                    tex_idx, poly_flags, aux_mat, aux_flags, lod_bias, lod_style = unpack('6i', f.read(24))
                    psk.materials.append(bytes_to_string(mat_name))
            
            elif section_name == b'REFSKELT':
                # Skeleton bones
                psk.bones = read_bones(f, data_count)
            
            elif section_name == b'RAWWEIGHTS':
                # Vertex weights: weight(f), point_index(i), bone_index(i)
                for _ in range(data_count):
                    weight, point_idx, bone_idx = unpack('fii', f.read(12))
                    psk.weights.append((point_idx, bone_idx, weight))
                psk.weights.sort(key=lambda x: x[0])
            
            elif section_name.startswith(b'EXTRAUV'):
                # PSKX: Extra UV channels
                psk.is_pskx = True
                extra_uvs = []
                for _ in range(data_count):
                    u, v = unpack('2f', f.read(8))
                    extra_uvs.append((u, v))
                psk.extra_uvs.append(extra_uvs)
            
            elif section_name == b'VTXNORMS':
                # PSKX: Vertex normals
                psk.is_pskx = True
                for _ in range(data_count):
                    x, y, z = unpack('3f', f.read(12))
                    psk.vertex_normals.append((x, y, z))
            
            elif section_name == b'VERTEXCOLOR':
                # PSKX: Vertex colors (RGBA, 1 byte each)
                psk.is_pskx = True
                for _ in range(data_count):
                    r, g, b, a = unpack('4B', f.read(4))
                    psk.vertex_colors.append((r / 255.0, g / 255.0, b / 255.0, a / 255.0))
            
            elif section_name == b'MRPHINFO':
                # PSKX: Morph target info
                psk.is_pskx = True
                for _ in range(data_count):
                    name = f.read(64)
                    vertex_count = unpack('i', f.read(4))[0]
                    psk.morph_infos.append((bytes_to_string(name), vertex_count))
            
            elif section_name == b'MRPHDATA':
                # PSKX: Morph target vertex data
                psk.is_pskx = True
                for _ in range(data_count):
                    # position_delta(3f), tangent_z_delta(3f), point_index(i)
                    px, py, pz, tx, ty, tz, point_idx = unpack('6fi', f.read(28))
                    psk.morph_data.append({
                        'position_delta': (px, py, pz),
                        'tangent_z_delta': (tx, ty, tz),
                        'point_index': point_idx
                    })
            
            else:
                # Unknown section, skip
                f.seek(data_size * data_count, os.SEEK_CUR)
    
    return psk


def read_bones(f, count):
    """Read bone data and build hierarchy"""
    raw_bones = []
    for _ in range(count):
        # VBone: name(64s), flags(i), children_count(i), parent_index(i),
        #        rotation(4f), location(3f), length(f), size(3f)
        name = bytes_to_string(f.read(64))
        flags, children_count, parent_index = unpack('3i', f.read(12))
        qx, qy, qz, qw = unpack('4f', f.read(16))
        px, py, pz = unpack('3f', f.read(12))
        length, sx, sy, sz = unpack('4f', f.read(16))
        
        rotation = mathutils.Quaternion((qw, qx, qy, qz))
        translation = mathutils.Vector((px, py, pz))
        
        # Parent reference (handle root bone case where parent_index might equal current index)
        parent = None
        if parent_index >= 0 and parent_index < len(raw_bones):
            parent = raw_bones[parent_index]
        
        raw_bones.append(RawEditBone(name, parent, rotation, translation))
    
    return raw_bones


# =============================================================================
# PSK Import Function
# =============================================================================

def pskimport(filepath, import_mesh=True, import_bones=True, import_extra_uvs=True,
              import_vertex_colors=True, import_vertex_normals=True, import_shape_keys=True):
    """
    Import PSK/PSKX file into Blender.
    
    Args:
        filepath: Path to the PSK/PSKX file
        import_mesh: Import mesh geometry
        import_bones: Import skeleton/armature
        import_extra_uvs: Import additional UV channels (PSKX)
        import_vertex_colors: Import vertex colors (PSKX)
        import_vertex_normals: Import custom vertex normals (PSKX)
        import_shape_keys: Import morph targets as shape keys (PSKX)
    """
    print(f"Importing PSK: {filepath}")
    
    psk = read_psk(filepath)
    obj_name = os.path.splitext(os.path.basename(filepath))[0]
    
    format_type = "PSKX (Extended)" if psk.is_pskx else "PSK (Legacy)"
    print(f"  Format: {format_type}")
    print(f"  Vertices: {len(psk.points)}, Faces: {len(psk.faces)}, "
          f"Bones: {len(psk.bones)}, Materials: {len(psk.materials)}")
    if psk.is_pskx:
        print(f"  Extra UVs: {len(psk.extra_uvs)}, Vertex Colors: {len(psk.vertex_colors) > 0}, "
              f"Normals: {len(psk.vertex_normals) > 0}, Morphs: {len(psk.morph_infos)}")
    
    # Create armature
    armature_obj = None
    if import_bones and psk.bones:
        armature_obj = create_armature(obj_name, psk.bones)
    
    # Create mesh
    mesh_obj = None
    if import_mesh:
        mesh_obj = create_mesh(obj_name, psk, import_extra_uvs, import_vertex_colors,
                               import_vertex_normals, import_shape_keys)
        
        # Setup vertex groups and weights
        if armature_obj and psk.weights:
            setup_vertex_weights(mesh_obj, armature_obj, psk.weights)
        
        # Parent mesh to armature
        if armature_obj:
            mesh_obj.parent = armature_obj
            modifier = mesh_obj.modifiers.new(name='Armature', type='ARMATURE')
            modifier.object = armature_obj
    
    bpy.context.view_layer.update()
    print("PSK import completed")
    
    return armature_obj, mesh_obj


def create_armature(name, bones):
    """Create armature from bone data"""
    armature_data = bpy.data.armatures.new(f"{name}_armature")
    armature_obj = bpy.data.objects.new(f"{name}_armature", armature_data)
    bpy.context.collection.objects.link(armature_obj)
    
    # Deselect all and select armature
    for obj in bpy.context.view_layer.objects:
        obj.select_set(False)
    armature_obj.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj
    
    # Enter edit mode to create bones
    bpy.ops.object.mode_set(mode='EDIT')
    
    for bone in bones:
        edit_bone = armature_data.edit_bones.new(bone.name)
        if bone.parent is not None:
            edit_bone.parent = armature_data.edit_bones[bone.parent.name]
        edit_bone.head = mathutils.Vector((0, 0, 0))
        edit_bone.tail = mathutils.Vector((0, 1, 0))
        edit_bone.matrix = bone.matrix
    
    bpy.ops.object.mode_set(mode='OBJECT')
    return armature_obj


def create_mesh(name, psk, import_extra_uvs, import_vertex_colors,
                import_vertex_normals, import_shape_keys):
    """Create mesh from PSK data using bmesh"""
    mesh_data = bpy.data.meshes.new(name)
    bm = bmesh.new()
    
    # Add vertices
    for point in psk.points:
        bm.verts.new(point)
    bm.verts.ensure_lookup_table()
    
    # Add faces
    invalid_faces = []
    for face_idx, face in enumerate(psk.faces):
        try:
            verts = [
                bm.verts[psk.wedges[face.wedge_indices[2]].point_index],
                bm.verts[psk.wedges[face.wedge_indices[1]].point_index],
                bm.verts[psk.wedges[face.wedge_indices[0]].point_index],
            ]
            bm_face = bm.faces.new(verts)
            bm_face.material_index = face.material_index
        except ValueError:
            # Duplicate or degenerate face
            invalid_faces.append(face_idx)
    
    if invalid_faces:
        print(f"  Warning: Skipped {len(invalid_faces)} invalid faces")
    
    bm.to_mesh(mesh_data)
    bm.free()
    
    # Add materials
    for mat_name in psk.materials:
        material = bpy.data.materials.get(mat_name) or bpy.data.materials.new(mat_name)
        mesh_data.materials.append(material)
    
    # Add primary UV layer
    uv_layer = mesh_data.uv_layers.new(name='UVMap')
    for face_idx, face in enumerate(psk.faces):
        if face_idx in invalid_faces:
            continue
        poly = mesh_data.polygons[face_idx - sum(1 for i in invalid_faces if i < face_idx)]
        for i, loop_idx in enumerate(poly.loop_indices):
            wedge_idx = face.wedge_indices[2 - i]  # Reverse order
            wedge = psk.wedges[wedge_idx]
            uv_layer.data[loop_idx].uv = (wedge.u, 1.0 - wedge.v)
    
    # Add extra UV layers (PSKX)
    if import_extra_uvs and psk.extra_uvs:
        for uv_idx, extra_uvs in enumerate(psk.extra_uvs):
            uv_layer = mesh_data.uv_layers.new(name=f'EXTRAUV{uv_idx}')
            for face_idx, face in enumerate(psk.faces):
                if face_idx in invalid_faces:
                    continue
                poly_idx = face_idx - sum(1 for i in invalid_faces if i < face_idx)
                poly = mesh_data.polygons[poly_idx]
                for i, loop_idx in enumerate(poly.loop_indices):
                    wedge_idx = face.wedge_indices[2 - i]
                    u, v = extra_uvs[wedge_idx]
                    uv_layer.data[loop_idx].uv = (u, 1.0 - v)
    
    # Add vertex colors (PSKX)
    if import_vertex_colors and psk.vertex_colors:
        color_attr = mesh_data.attributes.new(name='VERTEXCOLOR', type='FLOAT_COLOR', domain='CORNER')
        for face_idx, face in enumerate(psk.faces):
            if face_idx in invalid_faces:
                continue
            poly_idx = face_idx - sum(1 for i in invalid_faces if i < face_idx)
            poly = mesh_data.polygons[poly_idx]
            for i, loop_idx in enumerate(poly.loop_indices):
                wedge_idx = face.wedge_indices[2 - i]
                color_attr.data[loop_idx].color = psk.vertex_colors[wedge_idx]
    
    # Apply vertex normals (PSKX)
    if import_vertex_normals and psk.vertex_normals:
        mesh_data.polygons.foreach_set('use_smooth', [True] * len(mesh_data.polygons))
        normals = [tuple(n) for n in psk.vertex_normals]
        mesh_data.normals_split_custom_set_from_vertices(normals)
    else:
        mesh_data.shade_smooth()
    
    # Create mesh object
    mesh_obj = bpy.data.objects.new(name, mesh_data)
    bpy.context.collection.objects.link(mesh_obj)
    
    # Add shape keys (PSKX morphs)
    if import_shape_keys and psk.morph_infos:
        mesh_obj.shape_key_add(name='Basis', from_mix=False)
        morph_data_iter = iter(psk.morph_data)
        for morph_name, vertex_count in psk.morph_infos:
            shape_key = mesh_obj.shape_key_add(name=morph_name, from_mix=False)
            for _ in range(vertex_count):
                morph = next(morph_data_iter)
                point_idx = morph['point_index']
                delta = morph['position_delta']
                shape_key.data[point_idx].co += mathutils.Vector(delta)
    
    mesh_data.update()
    return mesh_obj


def setup_vertex_weights(mesh_obj, armature_obj, weights):
    """Setup vertex groups and apply weights"""
    # Create vertex groups for each bone
    for bone in armature_obj.data.bones:
        mesh_obj.vertex_groups.new(name=bone.name)
    
    # Apply weights
    for point_idx, bone_idx, weight in weights:
        if bone_idx < len(armature_obj.data.bones):
            bone_name = armature_obj.data.bones[bone_idx].name
            vgroup = mesh_obj.vertex_groups.get(bone_name)
            if vgroup:
                vgroup.add([point_idx], weight, 'ADD')


# =============================================================================
# PSA Reader and Import
# =============================================================================

def read_psa(filepath):
    """Read PSA animation file"""
    bone_names = []
    raw_actions = []
    raw_keys = []
    
    with open(filepath, 'rb') as f:
        # Header
        read_section_header(f)
        
        # Bones
        section_name, _, _, bone_count = read_section_header(f)
        for _ in range(bone_count):
            data = f.read(120)
            name = bytes_to_string(data[:64])
            bone_names.append(name)
        
        # Animations
        section_name, _, _, anim_count = read_section_header(f)
        for _ in range(anim_count):
            data = unpack('64s64s4i3f3i', f.read(64 + 64 + 4*4 + 3*4 + 3*4))
            raw_actions.append(RawAction(tuple(map(bytes_to_string, data))))
        
        # Keys
        section_name, _, _, key_count = read_section_header(f)
        for _ in range(key_count):
            px, py, pz, qx, qy, qz, qw, time = unpack('3f4ff', f.read(32))
            raw_keys.append(RawKey((px, py, pz), (qx, qy, qz, qw), time))
    
    return bone_names, raw_actions, raw_keys


def split_by_lengths(seq, num):
    """Split sequence by specified lengths"""
    it = iter(seq)
    out = [x for x in (list(itertools.islice(it, n)) for n in num) if x]
    remain = list(it)
    return out if not remain else out + [remain]


def calculate_bone_matrix(args):
    """Calculate pose bone matrix - can be run in parallel."""
    found_bone, armature_data, raw_action, frame_index, raw_keys = args
    bone_index, bone_name = found_bone
    
    key_index = ((raw_action.FirstRawFrame + frame_index) * raw_action.TotalBones) + bone_index
    position = raw_keys[key_index].Position
    rotation = raw_keys[key_index].Orientation

    rotation_matrix = rotation.to_matrix().to_4x4()
    translation_matrix = mathutils.Matrix.Translation(position)
    
    delta_matrix = right_hand_matrix @ (rotation_matrix @ translation_matrix) @ right_hand_matrix
    
    bone_data = armature_data.bones[bone_name]
    local_matrix = bone_data.matrix_local
    if bone_data.parent is not None:
        local_matrix = (bone_data.parent.matrix_local.inverted() @ local_matrix).inverted()
    
    matrix_basis = local_matrix @ delta_matrix.inverted()
    return (bone_name, matrix_basis)


def apply_bone_keyframes(armature_obj, bone_results):
    """Apply calculated matrices and insert keyframes - must run on main thread."""
    for bone_name, matrix_basis in bone_results:
        pose_bone = armature_obj.pose.bones[bone_name]
        pose_bone.matrix_basis = matrix_basis
        pose_bone.keyframe_insert("location")
        pose_bone.keyframe_insert("rotation_quaternion")


def psaimport(filepath, context):
    """Import PSA animation file"""
    print(f"Importing PSA: {filepath}")
    
    bone_names, raw_actions, raw_keys = read_psa(filepath)
    
    if bpy.ops.object.mode_set.poll():
        bpy.ops.object.mode_set(mode='OBJECT', toggle=False)
    
    # Find target armature
    armature_name = 'ArmObject'
    if hasattr(context.scene, 'udk_importarmatureselect') and context.scene.udk_importarmatureselect:
        if hasattr(context.scene, 'udkas_list') and len(context.scene.udkas_list) > 0:
            armature_list = context.scene.udkas_list
            armature_idx = context.scene.udkimportarmature_list_idx
            armature_name = bpy.data.objects[armature_list[armature_idx]].name
    
    armature_obj = bpy.data.objects.get(armature_name)
    if not armature_obj:
        print(f"  Error: Armature '{armature_name}' not found")
        return
    
    armature_obj.animation_data_create()
    
    print(f"  Bones: {len(bone_names)}, Actions: {len(raw_actions)}, Keys: {len(raw_keys)}")
    
    pool = ThreadPool()
    for raw_action in raw_actions:
        armature_obj.animation_data.action = bpy.data.actions.new(name=raw_action.Name)
        found_bones = [
            (idx, name) for idx, name in enumerate(bone_names[:raw_action.TotalBones])
            if name in armature_obj.pose.bones.keys()
        ]
        
        for frame_idx in range(raw_action.NumRawFrames):
            context.scene.frame_set(frame_idx + 1)
            # Calculate matrices in parallel
            calc_args = [(bone, armature_obj.data, raw_action, frame_idx, raw_keys) for bone in found_bones]
            bone_results = pool.map(calculate_bone_matrix, calc_args)
            # Apply results on main thread
            apply_bone_keyframes(armature_obj, bone_results)
        break  # Import first action only for now
    
    pool.close()
    pool.join()
    bpy.context.view_layer.update()
    context.scene.frame_set(0)
    print("PSA import completed")


# =============================================================================
# Input Validation
# =============================================================================

def getInputFilenamepsk(self, filename, importmesh, importbone, bDebugLogPSK, importmultiuvtextures):
    ext = os.path.splitext(filename)[1].lower()
    if ext not in ('.psk', '.pskx'):
        raise IOError("The selected file is not a *.psk or *.pskx file")
    pskimport(filename, importmesh, importbone)


def getInputFilenamepsa(self, filename, context):
    ext = os.path.splitext(filename)[1].lower()
    if ext != '.psa':
        raise IOError("The selected file is not a *.psa file")
    psaimport(filename, context)


# =============================================================================
# Blender Operators
# =============================================================================

class IMPORT_OT_psk(bpy.types.Operator):
    """Import Unreal PSK/PSKX skeleton mesh"""
    bl_idname = "import_scene.psk"
    bl_label = "Import PSK/PSKX"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_options = {'UNDO'}

    filepath: StringProperty(subtype='FILE_PATH')
    filter_glob: StringProperty(default="*.psk;*.pskx", options={'HIDDEN'})
    
    importmesh: BoolProperty(
        name="Import Mesh",
        description="Import mesh geometry",
        default=True,
    )
    importbone: BoolProperty(
        name="Import Bones",
        description="Import skeleton/armature",
        default=True,
    )
    importmultiuvtextures: BoolProperty(
        name="Import Extra UVs",
        description="Import additional UV channels (PSKX)",
        default=True,
    )
    bDebugLogPSK: BoolProperty(
        name="Debug Log",
        description="Enable debug logging",
        default=False,
    )
    unrealbonesize: FloatProperty(
        name="Bone Length",
        description="Display bone length",
        default=1,
        min=0.001,
        max=1000,
    )

    def execute(self, context):
        pskimport(self.filepath, self.importmesh, self.importbone,
                  self.importmultiuvtextures, True, True, True)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


class IMPORT_OT_psa(bpy.types.Operator):
    """Import Unreal PSA animation"""
    bl_idname = "import_scene.psa"
    bl_label = "Import PSA"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    filepath: StringProperty(subtype='FILE_PATH')
    filter_glob: StringProperty(default="*.psa", options={'HIDDEN'})

    def execute(self, context):
        psaimport(self.filepath, context)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


class Panel_UDKImport(bpy.types.Panel):
    bl_label = "Unreal Import"
    bl_idname = "OBJECT_PT_unreal_import"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "File I/O"

    def draw(self, context):
        layout = self.layout
        layout.operator(OBJECT_OT_PSKPath.bl_idname)
        layout.prop(context.scene, "udk_importarmatureselect")
        if context.scene.udk_importarmatureselect:
            layout.operator(OBJECT_OT_UDKImportArmature.bl_idname)
            layout.template_list("UI_UL_list", "udkimportarmature_list", 
                               context.scene, "udkimportarmature_list",
                               context.scene, "udkimportarmature_list_idx", rows=5)
        layout.operator(OBJECT_OT_PSAPath.bl_idname)


class OBJECT_OT_PSKPath(bpy.types.Operator):
    """Select PSK/PSKX file to import"""
    bl_idname = "object.pskpath"
    bl_label = "Import PSK/PSKX"

    filepath: StringProperty(subtype='FILE_PATH')
    filter_glob: StringProperty(default="*.psk;*.pskx", options={'HIDDEN'})
    
    importmesh: BoolProperty(name="Import Mesh", default=True)
    importbone: BoolProperty(name="Import Bones", default=True)
    importmultiuvtextures: BoolProperty(name="Import Extra UVs", default=True)
    bDebugLogPSK: BoolProperty(name="Debug Log", default=False)
    unrealbonesize: FloatProperty(name="Bone Length", default=1, min=0.001, max=1000)

    def execute(self, context):
        pskimport(self.filepath, self.importmesh, self.importbone,
                  self.importmultiuvtextures, True, True, True)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


class UDKImportArmaturePG(bpy.types.PropertyGroup):
    string: StringProperty()
    bexport: BoolProperty(default=False, name="Export", options={"HIDDEN"})
    bselect: BoolProperty(default=False, name="Select", options={"HIDDEN"})
    otype: StringProperty(name="Type")


class OBJECT_OT_PSAPath(bpy.types.Operator):
    """Select PSA file to import"""
    bl_idname = "object.psapath"
    bl_label = "Import PSA"

    filepath: StringProperty(name="PSA File Path", maxlen=1024, default="")
    filter_glob: StringProperty(default="*.psa", options={'HIDDEN'})

    def execute(self, context):
        psaimport(self.filepath, context)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


class OBJECT_OT_UDKImportArmature(bpy.types.Operator):
    """Update armature list"""
    bl_idname = "object.udkimportarmature"
    bl_label = "Update Armature List"

    def execute(self, context):
        my_objlist = context.scene.udkimportarmature_list
        
        # Find all armatures
        armatures = [obj for obj in context.scene.objects if obj.type == 'ARMATURE']
        
        # Add new armatures
        for arm in armatures:
            found = any(item.name == arm.name and item.otype == arm.type for item in my_objlist)
            if not found:
                item = my_objlist.add()
                item.name = arm.name
                item.bselect = arm.select_get()
                item.otype = arm.type
        
        # Remove deleted armatures
        to_remove = []
        for i, item in enumerate(my_objlist):
            found = any(arm.name == item.name and arm.type == item.otype for arm in armatures)
            if not found:
                to_remove.append(i)
        
        for i in reversed(to_remove):
            my_objlist.remove(i)
        
        return {'FINISHED'}

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

import os
import bpy
import material_pb2
import google.protobuf.text_format

# Image datablocks that are packed or procedurally generated carry no file
# extension, so map Blender's file_format enum onto one OpenImageIO can write.
EXTENSION_MAP = {
    'PNG':  '.png',
    'JPEG': '.jpg',
    'JPEG2000': '.jp2',
    'TARGA': '.tga',
    'TARGA_RAW': '.tga',
    'BMP':  '.bmp',
    'TIFF': '.tif',
    'OPEN_EXR': '.exr',
    'OPEN_EXR_MULTILAYER': '.exr',
    'HDR':  '.hdr',
    'WEBP': '.webp',
}


class MTL_OT_exporterCommon():
    '''Maps a Blender Principled BSDF material onto the engine's metallic-
    roughness material (BaseColorFactor/MetallicFactor/RoughnessFactor properties
    plus optional DiffuseMap/NormalMap/MetallicMap/RoughnessMap samplers).'''

    def __init__(self, filepath, as_text=False, texture_dir="textures"):
        self.filepath = filepath
        self.as_text = as_text
        # Directory (relative to the resource root) where the model exporter
        # stores textures; used to build the DiffuseMap path.
        self.texture_dir = texture_dir

    def find_principled(self, material):
        if not material.use_nodes or material.node_tree is None:
            return None
        for node in material.node_tree.nodes:
            if node.type == 'BSDF_PRINCIPLED':
                return node
        return None

    def find_base_color_image(self, principled):
        base_color = principled.inputs.get("Base Color")
        if base_color is not None and base_color.is_linked:
            from_node = base_color.links[0].from_node
            if from_node.type == 'TEX_IMAGE' and from_node.image is not None:
                return from_node.image
        return None

    def texture_path(self, image):
        basename = bpy.path.basename(image.filepath_raw) if image.filepath_raw else ""
        if not basename:
            basename = image.name
            if not os.path.splitext(basename)[1]:
                basename += EXTENSION_MAP.get(image.file_format, '.png')
        return self.texture_dir + '/' + basename

    def add_vector4(self, material_buffer, name, value):
        prop = material_buffer.property.add()
        prop.name = name
        prop.vector4.x = value[0]
        prop.vector4.y = value[1]
        prop.vector4.z = value[2]
        prop.vector4.w = value[3]

    def add_float(self, material_buffer, name, value):
        prop = material_buffer.property.add()
        prop.name = name
        prop.scalar_float = value

    def add_sampler(self, material_buffer, name, image):
        sampler = material_buffer.sampler.add()
        sampler.name = name
        sampler.image.path = self.texture_path(image)

    def find_input_image(self, principled, input_name):
        '''Return the Image datablock feeding a Principled input, following a
        Normal Map node's Color input for the Normal slot.'''
        if principled is None:
            return None
        inp = principled.inputs.get(input_name)
        if inp is None or not inp.is_linked:
            return None
        from_node = inp.links[0].from_node
        if from_node.type == 'TEX_IMAGE':
            return from_node.image
        # Normal is typically Principled.Normal <- Normal Map.Color <- Image.
        color_in = from_node.inputs.get("Color")
        if color_in is not None and color_in.is_linked:
            img_node = color_in.links[0].from_node
            if img_node.type == 'TEX_IMAGE':
                return img_node.image
        return None

    def scalar_input(self, principled, name, default):
        '''Flat value of a Principled scalar input, or the default when it is
        absent or texture-driven.'''
        if principled is None:
            return default
        inp = principled.inputs.get(name)
        if inp is None or inp.is_linked:
            return default
        return float(inp.default_value)

    def fill_material_buffer(self, material_buffer, material):
        principled = self.find_principled(material)
        base_image = self.find_base_color_image(principled) if principled is not None else None
        normal_image = self.find_input_image(principled, "Normal")
        metallic_image = self.find_input_image(principled, "Metallic")
        roughness_image = self.find_input_image(principled, "Roughness")

        # Base-colour factor: white (untinted) when a base-colour texture drives
        # the colour, otherwise the flat Base Color; alpha defaults to 1.
        if base_image is not None:
            base_color = (1.0, 1.0, 1.0, 1.0)
        elif principled is not None and principled.inputs.get("Base Color") is not None:
            c = principled.inputs["Base Color"].default_value
            base_color = (c[0], c[1], c[2], c[3] if len(c) > 3 else 1.0)
        else:
            base_color = (0.8, 0.8, 0.8, 1.0)

        # Metallic / roughness factors: a texture drives the value at full
        # strength (factor 1.0, since the engine multiplies factor * texel);
        # otherwise the flat scalar from the Principled BSDF.
        metallic = 1.0 if metallic_image is not None else self.scalar_input(principled, "Metallic", 0.0)
        roughness = 1.0 if roughness_image is not None else self.scalar_input(principled, "Roughness", 0.5)

        self.add_vector4(material_buffer, "BaseColorFactor", base_color)
        self.add_float(material_buffer, "MetallicFactor", metallic)
        self.add_float(material_buffer, "RoughnessFactor", roughness)

        if base_image is not None:
            self.add_sampler(material_buffer, "DiffuseMap", base_image)
        if normal_image is not None:
            self.add_sampler(material_buffer, "NormalMap", normal_image)
        if metallic_image is not None:
            self.add_sampler(material_buffer, "MetallicMap", metallic_image)
        if roughness_image is not None:
            self.add_sampler(material_buffer, "RoughnessMap", roughness_image)

    def run(self, material):
        material_buffer = material_pb2.MaterialMsg()
        self.fill_material_buffer(material_buffer, material)

        if self.as_text:
            text_path = self.filepath.replace('.mtl', '.txt')
            print("Writting", text_path, ".")
            out = open(text_path, "wt")
            out.write("AEONMTL\n")
            out.write(google.protobuf.text_format.MessageToString(material_buffer))
            out.close()
            print("Done.")
        else:
            print("Writting", self.filepath, ".")
            out = open(self.filepath, "wb")
            out.write(b'AEONMTL\x00')
            out.write(material_buffer.SerializeToString())
            out.close()
            print("Done.")


class MTL_OT_exporter(bpy.types.Operator):

    '''Exports a material to an AeonGames Material (MTL) file'''
    bl_idname = "export_material.mtl"
    bl_label = "Export AeonGames Material"

    filepath: bpy.props.StringProperty(subtype='FILE_PATH')
    material_name: bpy.props.StringProperty(
        name="Material",
        description="Name of the material datablock to export; empty uses the active object's active material",
        default=""
    )
    texture_dir: bpy.props.StringProperty(
        name="Texture Directory",
        description="Directory (relative to the resource root) holding textures, used for the DiffuseMap path",
        default="textures"
    )
    as_text: bpy.props.BoolProperty(
        name="Export as Text",
        description="Write the protobuf message as a human-readable text file (.txt) instead of the binary .mtl",
        default=False
    )

    @classmethod
    def poll(cls, context):
        # Always allow: the material is resolved in execute() either from the
        # explicit material_name property (model exporter) or from the active
        # context (interactive use). context.material is only present in the
        # Properties editor, so it must not be relied upon here.
        return True

    def resolve_material(self, context):
        if self.material_name:
            return bpy.data.materials.get(self.material_name)
        material = getattr(context, "material", None)
        if material is not None:
            return material
        active = context.active_object
        if active is not None:
            return active.active_material
        return None

    def execute(self, context):
        material = self.resolve_material(context)
        if material is None:
            self.report({'ERROR'}, "No material to export")
            return {'CANCELLED'}
        self.filepath = bpy.path.ensure_ext(self.filepath, ".mtl")
        exporter = MTL_OT_exporterCommon(
            self.filepath,
            as_text=self.as_text,
            texture_dir=self.texture_dir
        )
        exporter.run(material)
        return {'FINISHED'}

    def invoke(self, context, event):
        if not self.filepath:
            material = self.resolve_material(context)
            name = material.name if material is not None else "material"
            self.filepath = bpy.path.ensure_ext(
                os.path.dirname(bpy.data.filepath) + os.sep + name, ".mtl")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

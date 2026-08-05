// Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#version 450

#if defined(METAL)
#extension GL_EXT_nonuniform_qualifier : require
#elif defined(VULKAN)
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#else
#extension GL_ARB_bindless_texture : require
#endif

struct GpuMaterial
{
      uvec2 texture_refs[6];
      vec4  base_color_factor;
      float metallic_factor;
      float roughness_factor;
      float pad0;
      float pad1;
      vec4  emissive_factor;
};

const uint MATERIAL_CAPACITY = 4096u;
#if defined(METAL)
const uint BINDLESS_TEXTURE_CAPACITY = 16384u;
layout(set = 2, binding = 0) uniform sampler2D global_textures[BINDLESS_TEXTURE_CAPACITY];
layout(set = 7, binding = 0, std430) readonly buffer Bindless
{
      GpuMaterial materials[];
};
layout(location = 5) flat in uint vMaterialIndex;
#define MATERIAL_INDEX  ( vMaterialIndex < MATERIAL_CAPACITY ? vMaterialIndex : 0u )
#define MAT_REC         materials[MATERIAL_INDEX]
#define DIFFUSE_MAP     global_textures[nonuniformEXT ( MAT_REC.texture_refs[0].x )]
#elif defined(VULKAN)
layout(set = 2, binding = 0) uniform sampler2D global_textures[];
layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer MaterialRef
{
      GpuMaterial data[];
};
layout(push_constant) uniform MaterialPushConstant
{
      layout(offset = 72) MaterialRef Materials;
};
layout(location = 5) flat in uint vMaterialIndex;
#define MATERIAL_INDEX  ( vMaterialIndex < MATERIAL_CAPACITY ? vMaterialIndex : 0u )
#define MAT_REC         Materials.data[MATERIAL_INDEX]
#define DIFFUSE_MAP     global_textures[nonuniformEXT ( MAT_REC.texture_refs[0].x )]
#else
layout(binding = 4, std430) readonly buffer Bindless
{
      GpuMaterial materials[];
};
layout(location = 5) flat in uint vMaterialIndex;
#define MATERIAL_INDEX  ( vMaterialIndex < MATERIAL_CAPACITY ? vMaterialIndex : 0u )
#define MAT_REC         materials[MATERIAL_INDEX]
#define DIFFUSE_MAP     sampler2D ( MAT_REC.texture_refs[0] )
#endif

layout(location = 2) in vec2 CoordUV;
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 GNormalRough;
layout(location = 2) out vec4 GSpecWeight;

void main()
{
      FragColor = texture ( DIFFUSE_MAP, CoordUV ) * MAT_REC.base_color_factor;
      GNormalRough = vec4 ( 0.0, 0.0, 1.0, 1.0 );
      GSpecWeight = vec4 ( 0.0 );
}
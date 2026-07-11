#version 450

#ifndef VULKAN
#extension GL_ARB_shader_draw_parameters : require
#endif

// Directional shadow depth pass. Rasterizes static geometry into the shadow
// map using the light's world-space view-projection. Mirrors the vertex inputs
// of cluster_mark.vert / static_mesh.vert so the same static
// meshes bind unchanged. Skinned meshes fall back to their rest pose here.
#ifdef VULKAN
layout(set = 8, binding = 0, std140)
#else
layout(binding = 5, std140)
#endif
uniform ShadowParams
{
      mat4 light_view_projection; // world -> light clip space
      vec4 shadow_params;         // (texel_size, depth_bias, pcf_radius, enabled)
};

// Per-object model matrices, shared with the uber-pipeline. Indexed by the
// instance id so single and instanced draws bind unchanged.
#ifdef VULKAN
layout(set = 3, binding = 0, std430)
#else
layout(binding = 3, std430)
#endif
readonly buffer InstanceMatrices
{
      mat4 InstanceModelMatrices[];
};
#ifdef VULKAN
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceIndex]
#else
#define MODEL_MATRIX InstanceModelMatrices[gl_BaseInstanceARB + gl_InstanceID]
#endif

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

void main()
{
      gl_Position = light_view_projection * MODEL_MATRIX * vec4 ( VertexPosition, 1.0 );
#ifdef VULKAN
      // Matrix4x4::Ortho produces GL-style NDC z in [-1,1]; Vulkan clips to
      // [0,1]. Without this remap the near half of the light frustum would be
      // clipped out of the shadow map. (The perspective camera path tolerates
      // the raw matrix because its clipped near sliver is tiny; an orthographic
      // light projection would otherwise lose half its depth range.)
      gl_Position.z = ( gl_Position.z + gl_Position.w ) * 0.5;
#endif
}

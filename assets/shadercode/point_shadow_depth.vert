#version 450

// Point shadow depth pass. Rasterizes static geometry into one cube-face layer
// of the point shadow map. Unlike the directional/spot depth pass
// (shadow_depth.vert), which lets the hardware store projected NDC depth, this
// pass forwards the fragment's world position so the fragment shader can write
// LINEAR radial distance from the light. That makes the stored value
// view-independent and identical across all six faces, which the shading pass
// then compares against length(frag - light) / radius.
#ifdef VULKAN
layout(set = 8, binding = 0, std140)
#else
layout(binding = 5, std140)
#endif
uniform ShadowParams
{
      mat4 light_view_projection; // world -> this face's light clip space
      vec4 shadow_params;         // (light_x, light_y, light_z, radius)
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
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceID]
#endif

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

layout(location = 0) out vec3 world_position;

void main()
{
      vec4 world = MODEL_MATRIX * vec4 ( VertexPosition, 1.0 );
      world_position = world.xyz;
      gl_Position = light_view_projection * world;
#ifdef VULKAN
      // Matrix4x4::Perspective produces GL-style NDC z in [-1,1]; Vulkan clips
      // to [0,1]. Remap so the near half of the face frustum is not clipped.
      // (Only affects rasterization/clipping here; the stored depth value is
      // the linear distance written by the fragment shader, not this z.)
      gl_Position.z = ( gl_Position.z + gl_Position.w ) * 0.5;
#endif
}

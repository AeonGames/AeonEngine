#version 450

// Depth pre-pass mark stage (Phase R2). Rasterizes geometry only to record
// which clusters are occupied; mirrors the vertex inputs of
// diffuse_map_phong_no_skeleton.vert so the same static meshes bind unchanged.
#ifdef INSTANCED
#ifndef VULKAN
#extension GL_ARB_shader_draw_parameters : require
#endif
#endif

#if defined(VULKAN) && !defined(INSTANCED)
layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };
#endif
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
#ifndef VULKAN
      mat4 ModelMatrix;
#endif
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

// Per-instance model matrices for instanced draws (Track E). Under INSTANCED
// the per-node ModelMatrix is replaced by a lookup into this buffer, indexed by
// the instance id so a single draw can render many sibling nodes.
#ifdef INSTANCED
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
#else
#define MODEL_MATRIX ModelMatrix
#endif

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

// Eye-space position used by the fragment stage to resolve the cluster index.
layout(location = 0) out vec3 eyeCoords;

void main()
{
      eyeCoords = ( ViewMatrix * MODEL_MATRIX * vec4 ( VertexPosition, 1.0 ) ).xyz;
      gl_Position = ProjectionMatrix * ViewMatrix * MODEL_MATRIX * vec4 ( VertexPosition, 1.0 );
}

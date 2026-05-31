#version 450

// Depth pre-pass mark stage (Phase R2). Rasterizes geometry only to record
// which clusters are occupied; mirrors the vertex inputs of
// diffuse_map_phong_no_skeleton.vert so the same static meshes bind unchanged.
#ifdef VULKAN
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

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

// Eye-space position used by the fragment stage to resolve the cluster index.
layout(location = 0) out vec3 eyeCoords;

void main()
{
      eyeCoords = ( ViewMatrix * ModelMatrix * vec4 ( VertexPosition, 1.0 ) ).xyz;
      gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4 ( VertexPosition, 1.0 );
}

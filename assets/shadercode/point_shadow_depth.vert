#version 450

// Point shadow depth pass (single-pass, geometry-shader variant). Transforms
// each vertex to world space and forwards it; the projection into each of the
// six cube faces happens in point_shadow_depth.geom, which replicates the
// primitive across the six cube-map-array layers in one draw. The fragment
// shader writes normalized radial distance from the light (view-independent),
// so no per-face projection happens here.
//
// NOTE: a future optimization is to drop the geometry shader on Vulkan and use
// multiview (gl_ViewIndex) instead, projecting per view in this vertex shader.
// That requires per-renderer pipeline variants (no geometry stage on the Vulkan
// build), so for now both backends share this geometry-shader path.

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

layout(location = 0) out vec3 vWorldPosition;

void main()
{
      vWorldPosition = ( MODEL_MATRIX * vec4 ( VertexPosition, 1.0 ) ).xyz;
}

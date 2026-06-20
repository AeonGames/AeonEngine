#version 450

// Point shadow depth pass vertex shader (OpenGL geometry-shader path).
// Transforms each vertex to world space and forwards it; point_shadow_depth.geom
// projects it into the six cube faces and routes each to its array layer. On
// Vulkan this stage is replaced by point_shadow_depth_mv.vert, which projects
// per gl_ViewIndex directly via multiview (no geometry shader).

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

layout(location = 0) out vec3 v_world;

void main()
{
      v_world = ( MODEL_MATRIX * vec4 ( VertexPosition, 1.0 ) ).xyz;
}

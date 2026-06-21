#version 450

// Solid-color line pipeline used by the renderer's debug-geometry step
// (AABB wireframes, octree grid, camera frustums, markers). Sources its
// model matrix from the per-object storage buffer like every other
// graphics pipeline (Track E uber-pipeline), so it needs no instanced
// variant and shares the Proj/View Matrices UBO layout.
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

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
void main()
{
      gl_Position = ProjectionMatrix * ViewMatrix * MODEL_MATRIX * vec4 ( VertexPosition, 1.0 );
}

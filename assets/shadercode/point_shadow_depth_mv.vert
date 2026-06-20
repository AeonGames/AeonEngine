#version 450
#extension GL_EXT_multiview : require

// Point shadow depth pass vertex shader, Vulkan multiview variant. The shadow
// render pass has a six-bit view mask (one view per cube face); the GPU runs
// this shader once per view and gl_ViewIndex selects the face's view-projection.
// This renders all six faces in a single draw with no geometry shader (which the
// OpenGL build uses instead, see point_shadow_depth.geom). Vulkan-only: this
// source is selected for the "Vulkan" renderer, so it assumes Vulkan bindings
// and clip conventions.
layout(set = 8, binding = 0, std140)
uniform ShadowParams
{
      mat4 face_view_projection[6]; // POINT_SHADOW_FACES, indexed by gl_ViewIndex
      vec4 light_position_radius;   // xyz world pos, w radius
      vec4 face_params;             // x = base layer (unused under multiview)
};

layout(set = 3, binding = 0, std430)
readonly buffer InstanceMatrices
{
      mat4 InstanceModelMatrices[];
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

layout(location = 0) out vec3 world_position;

void main()
{
      vec4 world = InstanceModelMatrices[gl_InstanceIndex] * vec4 ( VertexPosition, 1.0 );
      world_position = world.xyz;
      vec4 clip = face_view_projection[gl_ViewIndex] * world;
      // Matrix4x4::Perspective produces GL-style NDC z in [-1,1]; Vulkan clips
      // to [0,1]. (Only affects clipping; stored depth is the linear distance
      // written by the fragment shader.)
      clip.z = ( clip.z + clip.w ) * 0.5;
      gl_Position = clip;
}

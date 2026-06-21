#version 450

// Analytic infinite ground grid for the renderer's debug-geometry step.
// A single full-screen triangle is unprojected to world space; the
// fragment stage intersects the eye ray with the z = 0 ground plane
// (this engine is +X right, +Y forward, +Z up) and draws the grid
// analytically, so the grid is truly infinite and needs no per-cell
// geometry. Shares the Proj/View Matrices UBO with every other pipeline.
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

layout(location = 0) in vec3 VertexPosition;
layout(location = 0) out vec3 NearPoint;
layout(location = 1) out vec3 FarPoint;
layout(location = 2) flat out vec3 CameraPosition;

vec3 Unproject ( vec2 aClipXY, float aClipZ, mat4 aInvViewProjection )
{
      vec4 world = aInvViewProjection * vec4 ( aClipXY, aClipZ, 1.0 );
      return world.xyz / world.w;
}

void main()
{
      mat4 inv_view_projection = inverse ( ProjectionMatrix * ViewMatrix );
      vec2 clip = VertexPosition.xy;
      // The fullscreen triangle is authored counter-clockwise in OpenGL's
      // Y-up window space. Vulkan's window space is Y-down, which flips the
      // winding to clockwise and gets it back-face culled, so mirror Y there.
      // gl_Position and the unprojected ray both use this same clip value, so
      // each fragment still resolves the world point under its own pixel.
#ifdef VULKAN
      clip.y = -clip.y;
#endif
      // Unproject the same clip XY at both depth extremes of this engine's
      // [-1, 1] projection to recover the eye ray in world space.
      NearPoint = Unproject ( clip, -1.0, inv_view_projection );
      FarPoint = Unproject ( clip, 1.0, inv_view_projection );
      CameraPosition = ( inverse ( ViewMatrix ) * vec4 ( 0.0, 0.0, 0.0, 1.0 ) ).xyz;
      gl_Position = vec4 ( clip, 0.0, 1.0 );
}

#version 450

// Camera matrices (shared engine block). The composite in the fragment shader
// reflects the environment along a per-pixel world-space view ray, built here
// from the inverse projection/view exactly as the skybox does.
#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

#ifdef VULKAN
#define VERTEX_ID gl_VertexIndex
#else
#define VERTEX_ID gl_VertexID
#endif

layout(location = 0) out vec2 CoordUV;
layout(location = 1) out vec3 WorldRayDir;

// Full-screen triangle generated from the vertex index alone (no vertex buffer):
// the three vertices land at (-1,-1), (3,-1) and (-1,3), which covers the whole
// screen, and their texture coordinates span [0,1] across the visible region.
// A tone-map/composite pass draws this with a 3-vertex, buffer-less draw call.
void main()
{
#ifdef VULKAN
      // Vulkan's framebuffer is y-down while the scene's y-flip lives in the
      // projection matrix, which this buffer-less triangle bypasses. Emit the
      // three vertices in reverse so the triangle is front-facing (CCW) under
      // the engine's baked back-face cull instead of being culled away.
      // Reversing the order preserves each position/UV pair, so the sampled
      // image stays upright.
      int vid = 2 - int ( VERTEX_ID );
#else
      int vid = int ( VERTEX_ID );
#endif
      vec2 uv = vec2 ( ( vid << 1 ) & 2, vid & 2 );
      CoordUV = uv;
      vec2 ndc = uv * 2.0 - 1.0;
      // Per-pixel world-space view ray (camera through this pixel); the fragment
      // shader reflects it about the surface normal to sample the environment.
      // Matches skybox.vert so reflections line up with the drawn sky.
      vec4 view_pos = inverse ( ProjectionMatrix ) * vec4 ( ndc, 1.0, 1.0 );
      view_pos /= view_pos.w;
      WorldRayDir = mat3 ( inverse ( ViewMatrix ) ) * view_pos.xyz;
      gl_Position = vec4 ( ndc, 0.0, 1.0 );
}

#version 450

// Camera matrices (shared engine block; the skybox only needs them to build a
// per-pixel world-space view ray). Same binding as the other passes.
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
#define VERTEX_ID gl_VertexIndex
#else
#define VERTEX_ID gl_VertexID
#endif

layout(location = 0) out vec3 WorldDir;

// Buffer-less fullscreen triangle (see tonemap.vert). On Vulkan the winding is
// reversed so the triangle stays front-facing under the engine's baked
// back-face cull (its y-flip lives in the projection matrix, which this
// direct-clip-space draw bypasses).
void main()
{
#ifdef VULKAN
      int vid = 2 - int ( VERTEX_ID );
#else
      int vid = int ( VERTEX_ID );
#endif
      vec2 uv = vec2 ( ( vid << 1 ) & 2, vid & 2 );
      vec2 ndc = uv * 2.0 - 1.0;
      // Unproject a far-plane clip point to view space, then rotate (no
      // translation) into world space to get this corner's view ray.
      vec4 view_pos = inverse ( ProjectionMatrix ) * vec4 ( ndc, 1.0, 1.0 );
      view_pos /= view_pos.w;
      WorldDir = mat3 ( inverse ( ViewMatrix ) ) * view_pos.xyz;
      // Sit just inside the far plane: cleared depth is the far value, so a
      // depth < far passes the LESS test and is overwritten by nearer geometry,
      // leaving the sky only where nothing else drew.
      gl_Position = vec4 ( ndc, 0.99999, 1.0 );
}

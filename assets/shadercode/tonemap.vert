#version 450

#ifdef VULKAN
#define VERTEX_ID gl_VertexIndex
#else
#define VERTEX_ID gl_VertexID
#endif

layout(location = 0) out vec2 CoordUV;

// Full-screen triangle generated from the vertex index alone (no vertex buffer):
// the three vertices land at (-1,-1), (3,-1) and (-1,3), which covers the whole
// screen, and their texture coordinates span [0,1] across the visible region.
// A tone-map pass draws this with a 3-vertex, buffer-less draw call.
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
      gl_Position = vec4 ( uv * 2.0 - 1.0, 0.0, 1.0 );
}

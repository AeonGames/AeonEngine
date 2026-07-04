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
      vec2 uv = vec2 ( ( VERTEX_ID << 1 ) & 2, VERTEX_ID & 2 );
      CoordUV = uv;
      gl_Position = vec4 ( uv * 2.0 - 1.0, 0.0, 1.0 );
}

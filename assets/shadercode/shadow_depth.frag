#version 450

// Directional shadow depth pass fragment shader. Only depth matters: the depth
// attachment is the shadow map. The Vulkan render pass carries a throwaway
// colour attachment (DONT_CARE) for render-pass compatibility with the main
// pipelines; the OpenGL shadow framebuffer has no colour attachment at all
// (glDrawBuffer(GL_NONE)). The colour value is discarded, but the ALPHA must be
// 1.0: the shared pipeline enables alpha-to-coverage, so an alpha of 0 would
// produce zero coverage and kill the fragment before its depth is written,
// leaving the shadow map empty on Vulkan.
layout(location = 0) out vec4 FragColor;

void main()
{
      FragColor = vec4 ( 1.0 );
}

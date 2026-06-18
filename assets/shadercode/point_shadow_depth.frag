#version 450

// Point shadow depth pass fragment shader. Writes the normalized radial
// distance from the light to this fragment as the depth value, so the shading
// pass can compare against the same metric (length(frag - light) / radius)
// regardless of which cube face stored the texel. Writing gl_FragDepth makes
// the comparison view-independent and backend-independent (no GL/Vulkan NDC
// remap), at the cost of early-Z; that is fine for a depth-only shadow pass.
//
// The colour attachment is a throwaway carried only for render-pass
// compatibility with the main pipelines (see shadow_depth.frag); its alpha must
// be 1.0 because the shared pipeline enables alpha-to-coverage.
#ifdef VULKAN
layout(set = 8, binding = 0, std140)
#else
layout(binding = 5, std140)
#endif
uniform ShadowParams
{
      mat4 light_view_projection;
      vec4 shadow_params; // (light_x, light_y, light_z, radius)
};

layout(location = 0) in vec3 world_position;
layout(location = 0) out vec4 FragColor;

void main()
{
      float dist = length ( world_position - shadow_params.xyz );
      gl_FragDepth = clamp ( dist / shadow_params.w, 0.0, 1.0 );
      FragColor = vec4 ( 1.0 );
}

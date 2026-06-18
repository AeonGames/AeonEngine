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
      mat4 face_view_projection[6]; // POINT_SHADOW_FACES
      vec4 light_position_radius;   // xyz world pos, w radius
      vec4 face_params;             // x = base cube-array layer
};

layout(location = 0) in vec3 gWorldPosition;
layout(location = 0) out vec4 FragColor;

void main()
{
      float dist = length ( gWorldPosition - light_position_radius.xyz );
      gl_FragDepth = clamp ( dist / light_position_radius.w, 0.0, 1.0 );
      FragColor = vec4 ( 1.0 );
}

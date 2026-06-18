#version 450

// Point shadow depth pass geometry shader (single-pass cube rendering). For each
// input triangle, emits it six times -- once per cube face -- routing each copy
// to its cube-map-array layer with gl_Layer and projecting it with that face's
// view-projection. This replaces the old six-draws-per-caster loop with a single
// draw per caster.
//
// The block is named "ShadowParams" so it binds through the same engine slot the
// directional/spot depth pass uses (its layout differs per pipeline; reflection
// is per-pipeline). gl_Layer = base_layer + face: base_layer is caster*6 on
// OpenGL (the whole cube-map array is attached, so layers are absolute) and 0 on
// Vulkan (a per-caster six-layer framebuffer is bound, so layers are relative).
//
// NOTE: a future Vulkan-multiview path would remove this geometry stage entirely
// (replicating per gl_ViewIndex in hardware); see point_shadow_depth.vert.
layout ( triangles ) in;
layout ( triangle_strip, max_vertices = 18 ) out;

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

layout(location = 0) in vec3 vWorldPosition[];
layout(location = 0) out vec3 gWorldPosition;

void main()
{
      int base_layer = int ( face_params.x );
      for ( int face = 0; face < 6; ++face )
      {
            gl_Layer = base_layer + face;
            for ( int i = 0; i < 3; ++i )
            {
                  gWorldPosition = vWorldPosition[i];
                  vec4 clip = face_view_projection[face] * vec4 ( vWorldPosition[i], 1.0 );
#ifdef VULKAN
                  // Matrix4x4::Perspective produces GL-style NDC z in [-1,1];
                  // Vulkan clips to [0,1]. Remap so the near half of the face
                  // frustum is not clipped (only affects clipping; the stored
                  // depth is the linear distance written by the fragment shader).
                  clip.z = ( clip.z + clip.w ) * 0.5;
#endif
                  gl_Position = clip;
                  EmitVertex();
            }
            EndPrimitive();
      }
}

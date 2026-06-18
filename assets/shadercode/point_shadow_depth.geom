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
            // Project the triangle into this face once, up front, so it can be
            // both frustum-culled and emitted without recomputing.
            vec4 clip0 = face_view_projection[face] * vec4 ( vWorldPosition[0], 1.0 );
            vec4 clip1 = face_view_projection[face] * vec4 ( vWorldPosition[1], 1.0 );
            vec4 clip2 = face_view_projection[face] * vec4 ( vWorldPosition[2], 1.0 );
            // Conservative per-face frustum cull: skip the face when all three
            // vertices lie outside the same clip plane (behind the eye, or past
            // one of the four side planes). Most triangles are visible on a
            // single cube face, so this avoids rasterizing each triangle on the
            // five faces it does not touch -- without it the geometry shader
            // would do roughly six times the shadow rasterization. The test uses
            // the raw GL clip (|x|,|y| <= w); the Vulkan z remap below does not
            // affect it.
            if ( clip0.w <= 0.0 && clip1.w <= 0.0 && clip2.w <= 0.0 )
            {
                  continue;
            }
            if ( clip0.x >  clip0.w && clip1.x >  clip1.w && clip2.x >  clip2.w )
            {
                  continue;
            }
            if ( clip0.x < -clip0.w && clip1.x < -clip1.w && clip2.x < -clip2.w )
            {
                  continue;
            }
            if ( clip0.y >  clip0.w && clip1.y >  clip1.w && clip2.y >  clip2.w )
            {
                  continue;
            }
            if ( clip0.y < -clip0.w && clip1.y < -clip1.w && clip2.y < -clip2.w )
            {
                  continue;
            }
            vec4 clip[3] = vec4[3] ( clip0, clip1, clip2 );
            gl_Layer = base_layer + face;
            for ( int i = 0; i < 3; ++i )
            {
                  gWorldPosition = vWorldPosition[i];
                  vec4 c = clip[i];
#ifdef VULKAN
                  // Matrix4x4::Perspective produces GL-style NDC z in [-1,1];
                  // Vulkan clips to [0,1]. Remap so the near half of the face
                  // frustum is not clipped (only affects clipping; the stored
                  // depth is the linear distance written by the fragment shader).
                  c.z = ( c.z + c.w ) * 0.5;
#endif
                  gl_Position = c;
                  EmitVertex();
            }
            EndPrimitive();
      }
}

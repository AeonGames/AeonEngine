#version 450

// Point shadow depth geometry shader (OpenGL single-pass cube path). For each
// input triangle, emits it once per cube face into that face's cube-map-array
// layer (gl_Layer = base_layer + face), projecting by that face's
// view-projection. Replaces the six-draws-per-caster loop with one draw. On
// Vulkan this stage is omitted entirely; multiview (point_shadow_depth_mv.vert)
// does the per-view projection in hardware instead.
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

layout(location = 0) in vec3 v_world[];
layout(location = 0) out vec3 world_position;

void main()
{
      int base_layer = int ( face_params.x );
      for ( int face = 0; face < 6; ++face )
      {
            vec4 clip0 = face_view_projection[face] * vec4 ( v_world[0], 1.0 );
            vec4 clip1 = face_view_projection[face] * vec4 ( v_world[1], 1.0 );
            vec4 clip2 = face_view_projection[face] * vec4 ( v_world[2], 1.0 );
            // Conservative per-face frustum cull: skip the face when all three
            // vertices lie outside the same clip plane, so each triangle is only
            // rasterized on the face(s) it touches instead of all six.
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
                  world_position = v_world[i];
                  vec4 c = clip[i];
#ifdef VULKAN
                  c.z = ( c.z + c.w ) * 0.5;
#endif
                  gl_Position = c;
                  EmitVertex();
            }
            EndPrimitive();
      }
}

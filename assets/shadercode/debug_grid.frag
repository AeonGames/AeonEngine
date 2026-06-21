#version 450

layout(location = 0) in vec3 NearPoint;
layout(location = 1) in vec3 FarPoint;
layout(location = 2) flat in vec3 CameraPosition;
layout(location = 0) out vec4 FragColor;

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
layout(set = 1, binding = 0, std140)
#else
layout(binding = 1, std140)
#endif
uniform Material
{
      vec4 GridColor;
      vec4 GridMajorColor;
      vec4 AxisXColor;
      vec4 AxisYColor;
      float CellSize;
      float MajorInterval;
      float FadeDistance;
};

// Distance, in pixels, from a coordinate to the nearest integer grid line
// (one unit = one cell). Uses screen-space derivatives so the measure is
// resolution independent.
float GridLineDistancePixels ( vec2 aCoord )
{
      vec2 derivative = fwidth ( aCoord );
      vec2 distance = abs ( fract ( aCoord - 0.5 ) - 0.5 ) / derivative;
      return min ( distance.x, distance.y );
}

void main()
{
      // Intersect the eye ray with the z = 0 ground plane.
      float t = -NearPoint.z / ( FarPoint.z - NearPoint.z );
      if ( t <= 0.0 )
      {
          discard;
      }
      vec3 world = NearPoint + t * ( FarPoint - NearPoint );

      // Fade the grid out with horizontal distance from the camera so the
      // far field does not turn into a solid moire sheet.
      float camera_distance = length ( world.xy - CameraPosition.xy );
      if ( camera_distance >= FadeDistance )
      {
          discard;
      }

      // Pick the highest-priority line covering this pixel: world axes first,
      // then major lines, then minor lines. Each is a crisp solid line of a
      // fixed pixel half-width; the pipeline is single-sample with no alpha
      // blending (Vulkan also enables alpha-to-coverage), so a fractional
      // alpha would dither into a stipple. Comparing pixel distances against
      // a half-width and emitting opaque pixels keeps every line uniform.
      const float minor_half_width = 0.5;
      const float major_half_width = 0.75;
      const float axis_half_width  = 1.0;

      vec2 pixel = fwidth ( world.xy );
      float x_axis_dist = abs ( world.y ) / pixel.y;
      float y_axis_dist = abs ( world.x ) / pixel.x;
      float major_dist = GridLineDistancePixels ( world.xy / ( CellSize * MajorInterval ) );
      float minor_dist = GridLineDistancePixels ( world.xy / CellSize );

      vec3 color;
      if ( x_axis_dist <= axis_half_width )
      {
          color = AxisXColor.rgb;
      }
      else if ( y_axis_dist <= axis_half_width )
      {
          color = AxisYColor.rgb;
      }
      else if ( major_dist <= major_half_width )
      {
          color = GridMajorColor.rgb;
      }
      else if ( minor_dist <= minor_half_width )
      {
          color = GridColor.rgb;
      }
      else
      {
          discard;
      }
      FragColor = vec4 ( color, 1.0 );

      // Write true scene depth so solid geometry occludes the grid. This
      // engine's projection emits [-1, 1] clip depth on both backends;
      // OpenGL maps that to the [0, 1] depth range, Vulkan uses it directly.
      vec4 clip = ProjectionMatrix * ViewMatrix * vec4 ( world, 1.0 );
      float ndc_depth = clip.z / clip.w;
#ifdef VULKAN
      gl_FragDepth = ndc_depth;
#else
      gl_FragDepth = ndc_depth * 0.5 + 0.5;
#endif
}

#version 450

// Equirectangular HDR environment map (linear radiance). Combined image sampler;
// on Vulkan it lands in the material "Samplers" set, which for this pipeline
// holds only this one texture.
#ifdef VULKAN
layout(set = 2, binding = 0)
#else
layout(binding = 0)
#endif
uniform sampler2D EnvironmentMap;

layout(location = 0) in vec3 WorldDir;
layout(location = 0) out vec4 FragColor;

const float PI = 3.14159265359;

// Map a world-space direction to equirectangular (lat-long) UVs. World up is
// +Z in this engine (view space is +Y forward, +Z up), so the polar angle is
// measured from Z and the azimuth around the X-Y plane.
vec2 direction_to_equirect ( vec3 dir )
{
      float u = atan ( dir.y, dir.x ) / ( 2.0 * PI ) + 0.5;
      float v = acos ( clamp ( dir.z, -1.0, 1.0 ) ) / PI;
      return vec2 ( u, v );
}

void main()
{
      vec3 dir = normalize ( WorldDir );
      // Output linear HDR radiance; the fullscreen tonemap pass exposes, tone
      // maps and sRGB-encodes it along with the rest of the scene.
      FragColor = vec4 ( texture ( EnvironmentMap, direction_to_equirect ( dir ) ).rgb, 1.0 );
}

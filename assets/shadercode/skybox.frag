#version 450

// Environment cube map (linear radiance). Combined image sampler; on Vulkan it
// lands in the material "Samplers" set, which for this pipeline holds only this
// one texture.
#ifdef VULKAN
layout(set = 2, binding = 0)
#else
layout(binding = 0)
#endif
uniform samplerCube EnvironmentMap;

layout(location = 0) in vec3 WorldDir;
layout(location = 0) out vec4 FragColor;

void main()
{
      vec3 dir = normalize ( WorldDir );
      // Output linear HDR radiance; the fullscreen tonemap pass exposes, tone
      // maps and sRGB-encodes it along with the rest of the scene. World up is
      // +Z; the cube is built with that convention so the sky is upright.
      FragColor = vec4 ( texture ( EnvironmentMap, dir ).rgb, 1.0 );
}

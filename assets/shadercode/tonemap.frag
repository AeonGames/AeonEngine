#version 450

// The linear HDR scene colour produced by the geometry pass, rendered to an
// off-screen RGBA16F target. This fullscreen pass resolves it to the display.
#ifdef VULKAN
layout(set = 0, binding = 0)
#else
layout(binding = 0)
#endif
uniform sampler2D HdrColor;

layout(location = 0) in vec2 CoordUV;
layout(location = 0) out vec4 FragColor;

// Overall exposure applied before tone mapping. 1.0 is neutral; raise to
// brighten, lower to darken. A future scene/camera value can drive this.
const float EXPOSURE = 1.0;

// Narkowicz's cheap ACES filmic curve: maps unbounded linear HDR radiance into
// a pleasing [0,1] range, rolling off bright highlights instead of clipping.
vec3 aces_tonemap ( vec3 x )
{
      const float a = 2.51;
      const float b = 0.03;
      const float c = 2.43;
      const float d = 0.59;
      const float e = 0.14;
      return clamp ( ( x * ( a * x + b ) ) / ( x * ( c * x + d ) + e ), 0.0, 1.0 );
}

// Encode linear colour to sRGB (approximate gamma 2.2) for the UNORM swapchain,
// which applies no hardware sRGB conversion.
vec3 linear_to_srgb ( vec3 c )
{
      return pow ( c, vec3 ( 1.0 / 2.2 ) );
}

void main()
{
      vec3 hdr = texture ( HdrColor, CoordUV ).rgb;
      vec3 color = aces_tonemap ( hdr * EXPOSURE );
      color = linear_to_srgb ( color );
      FragColor = vec4 ( color, 1.0 );
}

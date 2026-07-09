#version 450
#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#else
layout(binding = 1, std140)
#endif
uniform Material{
vec4 SolidColor;
};
layout ( location = 0 ) out vec4 FragColor;
layout ( location = 1 ) out vec4 GNormalRough;
layout ( location = 2 ) out vec4 GSpecWeight;

void main()
{
FragColor = SolidColor;
// Empty G-buffer values so the pipeline matches the 3-attachment main render
// pass (VUID-07609); debug wireframes have no meaningful normal/specular.
GNormalRough = vec4 ( 0.0, 0.0, 1.0, 0.0 );
GSpecWeight = vec4 ( 0.0 );
}

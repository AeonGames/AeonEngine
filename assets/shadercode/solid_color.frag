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

void main()
{
FragColor = SolidColor;
}

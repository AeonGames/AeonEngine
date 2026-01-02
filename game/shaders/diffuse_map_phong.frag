#version 450

#ifdef VULKAN
layout(set = 2, binding = 0)
#else
layout(binding = 0)
#endif
uniform sampler2D DiffuseMap;

layout(location = 0) in vec3 LightIntensity;
layout(location = 1) in vec2 CoordUV;
layout ( location = 0 ) out vec4 FragColor;

void main()
{
    if(gl_FrontFacing)
    {
        FragColor = vec4 ( texture(DiffuseMap,CoordUV) * vec4(LightIntensity, 1.0) );
    }
    else
    {
        FragColor = vec4 ( 1.0,0.5,0.5, 1.0 );
    }
}

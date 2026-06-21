#version 450
layout(location = 0) in vec3 LightIntensity;
layout ( location = 0 ) out vec4 FragColor;

void main()
{
if(gl_FrontFacing)
{
FragColor = vec4 ( LightIntensity, 1.0 );
}
else
{
FragColor = vec4 ( 1.0,0.5,0.5, 1.0 );
}
}

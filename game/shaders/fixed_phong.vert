#version 450
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
layout(binding = 0, std140) uniform Matrices
{
      mat4 ModelMatrix;
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 0) out vec3 LightIntensity;

void main()
{
    const vec3 Kd = vec3( 1.0f, 1.0f, 1.0f );
    const vec3 Ld = vec3( 1.0f, 0.38823529411f, 0.27843137254f );
    if ( length ( VertexNormal ) != 0 )
    {
        vec3 tnorm = normalize ( mat3(ViewMatrix) * mat3(ModelMatrix) * VertexNormal );
        vec4 eyeCoords = ViewMatrix * ModelMatrix * vec4 ( VertexPosition, 1.0 );
        vec3 s = normalize ( -eyeCoords.xyz );
        LightIntensity = Ld * Kd * max ( dot ( s, tnorm ), 0.0 );
    }
    else
    {
        LightIntensity = Ld;
    }
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(VertexPosition, 1.0);
}

#version 450
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
uniform Matrices
#endif
{
      mat4 ModelMatrix;
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

#ifdef VULKAN
layout(set = 2, binding = 0, std140)
#else
layout(binding = 2, std140)
#endif
uniform Skeleton
{
      mat4 skeleton[256];
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 5) in uvec4 VertexWeightIndices;
layout(location = 6) in vec4 VertexWeights;
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

    vec4 weighted_position =
        ( skeleton[VertexWeightIndices[0]] * VertexWeights[0] * vec4 ( VertexPosition, 1 ) ) +
        ( skeleton[VertexWeightIndices[1]] * VertexWeights[1] * vec4 ( VertexPosition, 1 ) ) +
        ( skeleton[VertexWeightIndices[2]] * VertexWeights[2] * vec4 ( VertexPosition, 1 ) ) +
        ( skeleton[VertexWeightIndices[3]] * VertexWeights[3] * vec4 ( VertexPosition, 1 ) );
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(weighted_position.xyz, 1.0);
}

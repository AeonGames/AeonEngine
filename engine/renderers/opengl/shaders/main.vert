#version 330

layout ( location = 0 ) in vec3 VertexPosition;
layout ( location = 1 ) in vec3 VertexNormal;
layout ( location = 2 ) in vec3 VertexTangent;
layout ( location = 3 ) in vec3 VertexBitangent;
layout ( location = 4 ) in vec2 VertexUV;
layout ( location = 5 ) in vec4 VertexWeightIndices;
layout ( location = 6 ) in vec4 VertexWeights;

out vec3 LightIntensity;

uniform vec3 LightPosition; // Light position in eye coords.
uniform vec3 Kd;            // Diffuse reflectivity
uniform vec3 Ld;            // Diffuse light intensity

uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;

void main()
{
    if ( length ( VertexNormal ) != 0 )
    {
        vec3 tnorm = normalize ( NormalMatrix * VertexNormal );
        vec4 eyeCoords = ModelViewMatrix * vec4 ( VertexPosition, 1.0 );
        vec3 s = normalize ( LightPosition - eyeCoords.xyz );
        LightIntensity = Ld * Kd * max ( dot ( s, tnorm ), 0.0 );
    }
    else
    {
        LightIntensity = Ld;
    }
    gl_Position = ModelViewProjectionMatrix * vec4 ( VertexPosition, 1.0 );
}
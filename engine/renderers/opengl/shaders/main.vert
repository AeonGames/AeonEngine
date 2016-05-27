#version 330

layout ( location = 0 ) in vec3 VertexPosition;
layout ( location = 1 ) in vec3 VertexNormal;

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

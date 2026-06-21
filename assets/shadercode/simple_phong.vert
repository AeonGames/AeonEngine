#version 450
layout(binding = 0, std140) uniform Matrices{
mat4 ProjectionMatrix;
mat4 ViewMatrix;
};
#ifdef VULKAN
layout(set = 3, binding = 0, std430)
#else
layout(binding = 3, std430)
#endif
readonly buffer InstanceMatrices{
mat4 InstanceModelMatrices[];
};
#ifdef VULKAN
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceIndex]
#else
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceID]
#endif
layout(binding = 1, std140) uniform Material{
  vec3 LightPosition;
  vec3 Kd;
  vec3 Ld;
};
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 0) out vec3 LightIntensity;
void main()
{
if ( length ( VertexNormal ) != 0 )
{
vec3 tnorm = normalize ( mat3(ViewMatrix) * mat3(MODEL_MATRIX) * VertexNormal );
vec4 eyeCoords = ViewMatrix * MODEL_MATRIX * vec4 ( VertexPosition, 1.0 );
vec3 s = normalize ( LightPosition - eyeCoords.xyz );
LightIntensity = Ld * Kd * max ( dot ( s, tnorm ), 0.0 );
}
else
{
LightIntensity = Ld;
}
gl_Position = ProjectionMatrix * ViewMatrix * MODEL_MATRIX * vec4(VertexPosition, 1.0);
}

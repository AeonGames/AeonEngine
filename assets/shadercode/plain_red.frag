#version 450
layout ( location = 0 ) out vec4 FragColor;
layout ( location = 1 ) out vec4 GNormalRough;
layout ( location = 2 ) out vec4 GSpecWeight;

void main()
{
    FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );
    // Empty G-buffer so the pipeline matches the 3-attachment main render pass.
    GNormalRough = vec4 ( 0.0, 0.0, 1.0, 0.0 );
    GSpecWeight = vec4 ( 0.0 );
}

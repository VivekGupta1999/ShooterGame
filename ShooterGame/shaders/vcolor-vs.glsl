#version 330

// vertex attributes
layout(location=0) in vec4 in_Position;
layout(location=1) in vec4 in_Color;

// vertex transform
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ModelViewMatrix;

// outputs to rasterizer
out vec4 var_Color;

void main(void)
{
    // output transformed vertex position
	gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * in_Position;

    // output vertex color to rasterizer
    var_Color = in_Color;
}

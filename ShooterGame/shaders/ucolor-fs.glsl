#version 330

// inputs from application
uniform vec4 u_Color;

// outputs to framebuffer
out vec4 out_Color;

void main(void)
{
	out_Color = u_Color;
}

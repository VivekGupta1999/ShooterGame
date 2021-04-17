#version 330

// inputs from rasterizer
in vec4 var_Color;

// outputs to framebuffer
out vec4 out_Color;

void main(void)
{
	out_Color = var_Color;
}

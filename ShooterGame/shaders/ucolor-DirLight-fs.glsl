#version 330

// inputs from application
uniform vec4 u_Color;

// input from rasterizer
in vec3 var_LightColor;		// interpolated per-vertex light color

// outputs to framebuffer
out vec4 out_Color;

void main(void)
{
	out_Color.rgb = u_Color.rgb * var_LightColor;
    out_Color.a = u_Color.a;
    //out_Color = u_Color;
    //out_Color = vec4(var_LightColor, 1);
}

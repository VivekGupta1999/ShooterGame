#version 330

// vertex attributes
layout(location=0) in vec4 in_Position;
layout(location=2) in vec3 in_Normal;

// transform
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat3 u_NormalMatrix;

// directional light info
uniform vec3 u_LightColor;
uniform vec3 u_LightDir;    // direction to light (in camera space!)

// outputs to rasterizer
out vec3 var_LightColor;

void main(void)
{
	// output transformed vertex position
	gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * in_Position;

	// can remove these normalizations if we're absolutely sure that normals and light directions are unit vectors
	vec3 N = normalize(u_NormalMatrix * in_Normal);		// transform surface normal
	vec3 L = normalize(u_LightDir);						// direction to light

	// compute diffuse lighting intensity
	float NdotL = max(dot(N, L), 0.2);

	// pass light color to rasterizer
	var_LightColor = NdotL * u_LightColor;

    //var_LightColor = u_LightColor;
    //var_LightColor = u_LightDir;
    //var_LightColor = in_Normal;
    //var_LightColor = N;
}

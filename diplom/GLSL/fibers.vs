#include GLSL/lighting.vs

varying vec4 color1;
varying vec3 TexCoord;

void main()
{
	color1 = gl_Color;

	TexCoord = gl_Vertex;

	prepareLight();

	gl_Position = ftransform();
}

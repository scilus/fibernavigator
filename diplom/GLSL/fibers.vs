#include GLSL/lighting.vs

varying vec3 color1;
varying vec3 TexCoord;

void main()
{
	color1 = gl_Color.rgb;
	TexCoord = gl_Vertex;

	prepareLight();

	gl_Position = ftransform();
}

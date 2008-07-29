#include GLSL/lighting.vs

varying vec3 color1;

void main()
{
	color1 = gl_Color;

	prepareLight();

	gl_Position = ftransform();
}

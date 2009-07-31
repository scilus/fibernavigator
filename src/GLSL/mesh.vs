#include lighting.vs

varying vec4 myColor;
varying vec4 VaryingTexCoord0;

void main()
{
	// pass the color to the fragment shader
	myColor = gl_Color;
	VaryingTexCoord0 = gl_Vertex;

	prepareLight();

	gl_Position = ftransform();
}

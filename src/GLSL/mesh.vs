#include lighting.vs

varying vec4 myColor;
varying vec4 VaryingTexCoord0;

void main()
{
    gl_FrontColor = gl_Color;
    gl_BackColor = gl_Color;

    myColor = gl_Color;
	VaryingTexCoord0 = gl_Vertex;

	prepareLight();

	gl_Position = ftransform();
}

#include lighting.vs

varying float flag;
varying vec4 myColor;

void main()
{
	// pass the color to the fragment shader
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;
	myColor = gl_Color;

	prepareLight();
	gl_TexCoord[0].xyz = gl_Vertex.xyz;

	gl_Position = ftransform();
}

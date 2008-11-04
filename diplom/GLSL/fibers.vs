#include GLSL/lighting.vs

void main() {
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;

	gl_TexCoord[0].xyz = gl_Vertex.xyz;

	prepareLight();

	gl_Position = ftransform();
}

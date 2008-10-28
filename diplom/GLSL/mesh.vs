#include GLSL/lighting.vs

void main()
{
	prepareLight();
	gl_TexCoord[0].xyz = gl_Vertex;
    gl_Position = ftransform();
}

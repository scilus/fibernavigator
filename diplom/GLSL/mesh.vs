#include lighting.vs

varying float flag;

void main()
{
	prepareLight();
	gl_TexCoord[0].xyz = gl_Vertex.xyz;

	gl_Position = ftransform();
}

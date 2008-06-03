uniform int dimX;
uniform int dimY;
uniform int dimZ;

varying vec3 TexCoord;

void main()
{
	TexCoord = gl_Vertex.xyz;
	
	gl_Position = ftransform();
}

varying vec3 TexCoord;

void main()
{
	prepareLight();
	TexCoord = gl_Vertex;
    gl_Position = ftransform();
}

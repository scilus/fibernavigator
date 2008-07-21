varying vec3 TexCoord;

void main()
{
	TexCoord = gl_MultiTexCoord0.xyz;
	gl_Position = ftransform();
}

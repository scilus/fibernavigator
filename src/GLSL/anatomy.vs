varying vec4 VaryingTexCoord0;

void main()
{
    VaryingTexCoord0 = gl_MultiTexCoord0;
	gl_Position = ftransform();
}

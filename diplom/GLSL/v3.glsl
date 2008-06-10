varying vec3 normal;
varying vec4 color;

void main()
{
	normal = gl_Normal;
	color = gl_Color;
	
	gl_Position = ftransform();
}

varying vec3 normal;
varying vec4 color;
uniform bool useNormals;

void main()
{    
	if (useNormals)
   		gl_FragColor = vec4(normal, 1.0);
   	else
   		gl_FragColor = color;
}

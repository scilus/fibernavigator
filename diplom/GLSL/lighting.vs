varying vec3 normal;
varying vec4 vertex;
varying vec4 half;

void prepareLight()
{
	/* Calculate the normal */
	normal = normalize(gl_NormalMatrix * gl_Normal);

	/* Transform the vertex position to eye space */
	vertex = vec4(gl_ModelViewMatrix * gl_Vertex);
	half = vec4(gl_ModelViewMatrix * gl_LightSource[0].position);
}


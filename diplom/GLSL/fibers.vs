varying vec3 color1;

void main()
{
	color1 = gl_Color;

	/* Calculate the normal */
	normal = normalize(gl_NormalMatrix * gl_Normal);


	/* Transform the vertex position to eye space */
	vertex = vec4(gl_ModelViewMatrix * gl_Vertex);
	half = vec4(gl_ModelViewMatrix * gl_LightSource[0].position);

	gl_Position = ftransform();
}

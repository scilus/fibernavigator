varying vec3 color1;
varying vec3 color2;

void main()
{
	color1 = gl_Color;
	color2 = gl_Normal;

	/* Calculate the normal */
	normal = normalize(gl_NormalMatrix * gl_Normal);


	/* Transform the vertex position to eye space */
	vertex = vec4(gl_ModelViewMatrix * gl_Vertex);

	vec3 L = normalize(gl_LightSource[0].position.xyz - vertex.xyz);

	half = vec3(L + vec3(0.0, 0.0, 1.0));

	gl_Position = ftransform();
}

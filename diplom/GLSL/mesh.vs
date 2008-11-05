#include GLSL/lighting.vs

varying float flag;

void main()
{
	prepareLight();
	gl_TexCoord[0].xyz = gl_Vertex.xyz;


	vec3 n = (gl_ModelViewProjectionMatrix * vec4(gl_Normal,0.)).xyz;
	vec3 v = (gl_ModelViewMatrix * vec4(0., 0., -1., 0.)).xyz;

	flag = dot(v, n);

	gl_Position = ftransform();
}

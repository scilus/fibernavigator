uniform sampler3D tex0;
uniform bool showMesh;

varying vec3 TexCoord;

void main()
{
	vec4 col;
	col = vec4(1.0);
	
	gl_FragColor = col;
}
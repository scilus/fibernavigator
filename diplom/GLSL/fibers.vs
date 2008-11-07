varying vec3 N, L;
varying vec4 myColor;

void main() {
	gl_TexCoord[0].xyz = gl_Vertex.xyz;

	N = gl_NormalMatrix * gl_Normal;

	vec4 V = gl_ModelViewMatrix * gl_Vertex;

	L = gl_LightSource[0].position.xyz - V.xyz;

	myColor = normalize(abs(gl_Color));

	gl_Position = ftransform();
}

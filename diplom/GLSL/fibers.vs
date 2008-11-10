varying vec3 normal;
varying vec3 position;

varying vec4 myColor;

void main() {
	gl_TexCoord[0].xyz = gl_Vertex.xyz;
	//
	myColor = abs(gl_Color);

	gl_Position = ftransform();

	normal = gl_NormalMatrix * gl_Normal;
	position = (gl_ModelViewMatrix * gl_Vertex).xyz;
}

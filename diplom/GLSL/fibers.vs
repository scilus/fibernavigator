#include lighting.vs

varying vec4 myColor;

void main() {
	gl_TexCoord[0].xyz = gl_Vertex.xyz;

	myColor = normalize(abs(gl_Color));
	prepareLight();

	gl_Position = ftransform();
}

varying vec4 myColor;

void main() {
	gl_TexCoord[0].xyz = gl_Vertex.xyz;

	myColor = abs(gl_Color);

	gl_Position = ftransform();
}

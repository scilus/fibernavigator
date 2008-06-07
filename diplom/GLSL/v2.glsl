varying vec3 normal;
varying vec3 vertex;
varying vec3 TexCoord;

void main()
{
	TexCoord = gl_Vertex;

    /* Calculate the normal */
    normal = normalize(gl_NormalMatrix * gl_Normal);
   
    /* Transform the vertex position to eye space */
    vertex = vec3(gl_ModelViewMatrix * gl_Vertex);
       
    gl_Position = ftransform();
}     
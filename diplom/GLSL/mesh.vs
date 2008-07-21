varying vec3 normal;
varying vec4 vertex;
varying vec4 half;
varying vec3 TexCoord;

void main()
{
	TexCoord = gl_Vertex;

    /* Calculate the normal */
    normal = normalize(gl_NormalMatrix * gl_Normal);
   
    /* Transform the vertex position to eye space */
    vertex = vec4(gl_ModelViewMatrix * gl_Vertex);
    half = vec4(gl_ModelViewMatrix * gl_LightSource[0].position);
       
    gl_Position = ftransform();
}     
attribute float  color;
uniform   int    displayControl; //==1 swapZ, else normal
uniform   mat3   tensorMatrix;
uniform   vec3   axisFlip, lightPosition, offset;
varying   float  texturePosition;
varying   vec3   lightDir, normal;
varying   vec3 	 vertexPos;

void main()
{	
	// We don't want to modify input variables
	vec3 tempVertex = gl_Vertex.xyz;

	// This will happen when we are drawing the second half of the tensor.
	if( displayControl == 1 ){
		tempVertex[2] *= -1.0;		
	}
	
	// Deform the vertex to its ellipsoid position.
	// Matrix * Vector will threat the vector as a column-vector ( OpenGL  standard )
	// Vector * Matrix will threat the vector as a row-vector 	 ( DirectX standard )
	tempVertex = tensorMatrix * tempVertex;

	// Set the correct flip to the tensor.
	tempVertex *= axisFlip;

	// Set the normal of this point for the lighting.
	normal = normalize( tempVertex );

	// This will color the pixel depending on its position.
	vertexPos = abs(tempVertex * color);
	
	// Place the tensor in the center of its voxel.
	tempVertex += offset;

	// Compute the light's direction.
	lightDir = normalize( lightPosition );

	// This value will be passed to the fragment shader to read the color on the texture.
	texturePosition = color;

	// We need the 4th value
	vec4 tempVertex4; 
	tempVertex4[0] = tempVertex[0];
	tempVertex4[1] = tempVertex[1];
	tempVertex4[2] = tempVertex[2];
	tempVertex4[3] =  gl_Vertex[3];
	
	gl_Position = gl_ModelViewProjectionMatrix * tempVertex4;
}
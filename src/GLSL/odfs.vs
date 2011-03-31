attribute float radius;
uniform vec2    radiusMinMax;
uniform vec3  	offset, axisFlip;
uniform int   	mapOnSphere, swapRadius;
varying float 	texturePosition;
varying vec3 	vertexPos;

void main()
{
	// We don't want to modify input variables
	vec3 tempVertex = gl_Vertex.xyz;
	float newRadius = radius;

	// Normalizing the newRadius.
	if( radiusMinMax[0] != radiusMinMax[1] )
		newRadius = ( newRadius - radiusMinMax[0] ) / ( radiusMinMax[1] - radiusMinMax[0] );
			
	// The color of the vertex is set from the newRadius.
	texturePosition = newRadius;
	
	// This will color the pixel depending on its position.
	vertexPos = abs( tempVertex * axisFlip * newRadius );

	// Since we cannot pass boolean uniform values, we pass an int (1 = GL_TRUE and 0 = GL_FALSE).
	if( mapOnSphere == 1 )
		newRadius = 1.0;
	
	// This will happen when we are drawing the second half of the odf.
	if( swapRadius == 1 )
		newRadius *= -1.0;

	// Set the correct flip and apply the newRadius modifier to this odf.
	tempVertex *= axisFlip * newRadius;
	
	// Place the odf in the center of its voxel.
	tempVertex += offset;

	// We need the 4th value
	vec4 tempVertex4; 
	tempVertex4[0] = tempVertex[0];
	tempVertex4[1] = tempVertex[1];
	tempVertex4[2] = tempVertex[2];
	tempVertex4[3] =  gl_Vertex[3];
	
	gl_Position = gl_ModelViewProjectionMatrix * tempVertex4;
}
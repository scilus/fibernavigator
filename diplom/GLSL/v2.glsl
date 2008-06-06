uniform int dimX;
uniform int dimY;
uniform int dimZ;
uniform int quadrant;

varying int discardIt;

void main()
{
	vec4 pos = gl_Vertex;
	discardIt = 0;
	
	if (quadrant == 1 &&
		pos.x > dimX &&
		pos.y > dimY &&
		pos.z > dimZ) discardIt = 1; 
	if (quadrant == 2 &&
		pos.x > dimX &&
		pos.y > dimY &&
		pos.z < dimZ) discardIt = 1;
	if (quadrant == 3 &&
		pos.x > dimX &&
		pos.y < dimY &&
		pos.z < dimZ) discardIt = 1;
	if (quadrant == 4 &&
		pos.x > dimX &&
		pos.y < dimY &&
		pos.z > dimZ) discardIt = 1;
	if (quadrant == 5 &&
		pos.x < dimX &&
		pos.y < dimY &&
		pos.z > dimZ) discardIt = 1;
	if (quadrant == 6 &&
		pos.x < dimX &&
		pos.y < dimY &&
		pos.z < dimZ) discardIt = 1;
	if (quadrant == 7 &&
		pos.x < dimX &&
		pos.y > dimY &&
		pos.z < dimZ) discardIt = 1;
	if (quadrant == 8 &&
		pos.x < dimX &&
		pos.y > dimY &&
		pos.z > dimZ) discardIt = 1;
		
	gl_Position = ftransform();
}

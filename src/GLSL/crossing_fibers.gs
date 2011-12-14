// these lines enable the geometry shader support.
#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float xMin, xMax, yMin, yMax, zMin, zMax;

void main( void )
{
	for( int i = 0 ; i < gl_VerticesIn ; i++ )
	{
    
        gl_FrontColor = gl_FrontColorIn[i];
        gl_Position = gl_ModelViewProjectionMatrix * gl_PositionIn[i];
        EmitVertex();
    }
    EndPrimitive();
}

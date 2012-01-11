// these lines enable the geometry shader support.
#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float xMin, xMax, yMin, yMax, zMin, zMax;

void main( void )
{
    for( int i = 0 ; i < gl_VerticesIn ; ++i )
    {
	gl_FrontColor = gl_FrontColorIn[i];
        gl_Position = gl_ModelViewProjectionMatrix * gl_PositionIn[i];

        if ( xMax > gl_PositionIn[i].x && xMin < gl_PositionIn[i].x )
        {
            EmitVertex();
        }
        else if ( yMax > gl_PositionIn[i].y && yMin < gl_PositionIn[i].y )
        {
            EmitVertex();
        }
        else if ( zMax > gl_PositionIn[i].z &&zMin < gl_PositionIn[i].z )
        {
            EmitVertex();
        }
    }
}

// these lines enable the geometry shader support.
#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float xMin, xMax, yMin, yMax, zMin, zMax;
uniform bool axialShown, coronalShown, sagittalShown;

void main( void )
{
    bool primitiveStarted = false;
    
	for( int i = 0 ; i < gl_VerticesIn ; i++ )
	{
        if ( sagittalShown && xMin <= gl_PositionIn[i].x && xMax >= gl_PositionIn[i].x )
        {
            primitiveStarted = true;
            gl_FrontColor = gl_FrontColorIn[i];
            gl_Position = gl_ModelViewProjectionMatrix * gl_PositionIn[i];
            EmitVertex();
        }
        else if ( coronalShown && yMin <= gl_PositionIn[i].y && yMax >= gl_PositionIn[i].y )
        {
            primitiveStarted = true;
            gl_FrontColor = gl_FrontColorIn[i];
            gl_Position = gl_ModelViewProjectionMatrix * gl_PositionIn[i];
            EmitVertex();
        }
        else if ( axialShown && zMin <= gl_PositionIn[i].z && zMax >= gl_PositionIn[i].z )
        {
            primitiveStarted = true;
            gl_FrontColor = gl_FrontColorIn[i];
            gl_Position = gl_ModelViewProjectionMatrix * gl_PositionIn[i];
            EmitVertex();
        }
        else if ( primitiveStarted )
        {
            EndPrimitive();
			primitiveStarted = false;
        }
    }
}

#include functions.fs

void main()
{
	float v = gl_TexCoord[0].x;
	vec4 c = vec4(1.0);
	if ( useColorMap == 1 )
		c.rgb  = colorMap1( v );
	else if ( useColorMap == 2 )
		c.rgb  = colorMap2( v );
	else if ( useColorMap == 3 )
		c.rgb  = colorMap3( v );
	else if ( useColorMap == 4 )
		c.rgb  = colorMap4( v );
	else
		c.rgb = defaultColorMap( v );

	gl_FragColor = c;
}

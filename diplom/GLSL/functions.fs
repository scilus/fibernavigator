vec3 defaultColorMap( float value )
{
    value *= 5.0;
	vec3 color;

	if( value < 0.0 )
		color = vec3( 0.0, 0.0, 0.0 );
    else if( value < 1.0 )
		color = vec3( 0.0, value, 1.0 );
	else if( value < 2.0 )
		color = vec3( 0.0, 1.0, 2.0-value );
    else if( value < 3.0 )
		color =  vec3( value-2.0, 1.0, 0.0 );
    else if( value < 4.0 )
		color = vec3( 1.0, 4.0-value, 0.0 );
    else if( value <= 5.0 )
		color = vec3( 1.0, 0.0, value-4.0 );
    else
		color =  vec3( 1.0, 0.0, 1.0 );
	return color;
}

void lookupTex(inout vec4 col, in int type, in sampler3D tex, in float threshold, in float alpha)
{
	vec3 col1 = vec3(0.0);

	col1 = clamp( texture3D(tex, gl_TexCoord[0].xyz), 0.0, 1.0);
	if ( type == 3 )
	{
		if ( col1. r - threshold <= 0.0) 
			return;
		col1 = defaultColorMap( col1.r );
	}
	if ( (length(col1) - threshold) > 0.1)
	{
		col.rgb = ((1.0 - alpha) * col.rgb) + (alpha * col1.rgb);
	}
	col.a += clamp (( (col.r*3.0) + (col.g*3.0) + (col.b*3.0) ), 0.0, 1.0) - threshold;
}

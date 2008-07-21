uniform sampler3D texes[10];
uniform bool show[10];
uniform float threshold[10];
uniform int type[10];
uniform int countTextures;

varying vec3 TexCoord;

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

void lookupTex(inout vec4 col, in int type, in sampler3D tex, in float threshold)
{
	vec3 col1;
	if (type == 3)
	{
		col1.r = clamp( texture3D(tex, TexCoord).r, 0.0, 1.0);
		
		if (col1.r - threshold > 0.0)
		{
			col.rgb = defaultColorMap( col1.r);
		}
	}	
	if (type == 1 || type == 2 || type == 4)
	{
		col1.r = clamp( texture3D(tex, TexCoord).r, 0.0, 1.0);
		col1.g = clamp( texture3D(tex, TexCoord).g, 0.0, 1.0);
		col1.b = clamp( texture3D(tex, TexCoord).b, 0.0, 1.0);
		
		if ( ((col1.r + col1.g + col1.b) / 3.0 - threshold) > 0.0)
		{
			col.rgb = col1.rgb;
		}
	}
	col.a += clamp ((col.r + col.g + col.b)/3.0 - threshold, 0.0, 1.0);
}


void main()
{
	vec4 col = vec4(0.0, 0.0, 0.0, 0.0);
	
	for (int i = 9 ; i > -1 ; i--)
	{
		if (show[i]) lookupTex(col, type[i], texes[i], threshold[i]);
	}

	col = clamp(col, 0.0, 1.0);
	gl_FragColor = col;
}

uniform sampler3D tex0, tex1, tex2, tex3, tex4;
uniform sampler3D tex5, tex6, tex7, tex8, tex9;

uniform bool show0, show1, show2, show3, show4;
uniform bool show5, show6, show7, show8, show9;

uniform float threshold0, threshold1, threshold2, threshold3;
uniform float threshold4, threshold5, threshold6, threshold7;
uniform float threshold8, threshold9;

uniform int type0, type1, type2, type3, type4;
uniform int type5, type6, type7, type8, type9;

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
	
	if (show9) lookupTex(col, type9, tex9, threshold9);
	if (show8) lookupTex(col, type8, tex8, threshold8);
	if (show7) lookupTex(col, type7, tex7, threshold7);
	if (show6) lookupTex(col, type6, tex6, threshold6);
	if (show5) lookupTex(col, type5, tex5, threshold5);
	if (show4) lookupTex(col, type4, tex4, threshold4);
	if (show3) lookupTex(col, type3, tex3, threshold3);
	if (show2) lookupTex(col, type2, tex2, threshold2);
	if (show1) lookupTex(col, type1, tex1, threshold1);
	if (show0) lookupTex(col, type0, tex0, threshold0);
	
	col = clamp(col, 0.0, 1.0);
	gl_FragColor = col;
}

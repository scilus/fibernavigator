uniform int countTextures;

uniform sampler3D tex0;
uniform sampler3D tex1;
uniform sampler3D tex2;
uniform sampler3D tex3;
uniform sampler3D tex4;
uniform sampler3D tex5;
uniform sampler3D tex6;
uniform sampler3D tex7;
uniform sampler3D tex8;
uniform sampler3D tex9;

uniform bool showTex0;
uniform bool showTex1;
uniform bool showTex2;
uniform bool showTex3;
uniform bool showTex4;
uniform bool showTex5;
uniform bool showTex6;
uniform bool showTex7;
uniform bool showTex8;
uniform bool showTex9;

uniform float thresholdTex0;
uniform float thresholdTex1;
uniform float thresholdTex2;
uniform float thresholdTex3;
uniform float thresholdTex4;
uniform float thresholdTex5;
uniform float thresholdTex6;
uniform float thresholdTex7;
uniform float thresholdTex8;
uniform float thresholdTex9;

uniform int typeTex0;
uniform int typeTex1;
uniform int typeTex2;
uniform int typeTex3;
uniform int typeTex4;
uniform int typeTex5;
uniform int typeTex6;
uniform int typeTex7;
uniform int typeTex8;
uniform int typeTex9;

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

void main()
{
	vec4 col = vec4(0);
	vec4 col1;
	
	if (showTex2)
	{
		if (typeTex2 == 3)
		{
			col1.r = clamp( texture3D(tex2, TexCoord).r, 0.0, 1.0);
			
			if (col1.r - thresholdTex2 > 0.0)
			{
				col.rgb = defaultColorMap( col1.r);
			}
		}	
		if (typeTex2 == 1 || typeTex2 == 2 || typeTex2 == 4)
		{
			col1.r = clamp( texture3D(tex2, TexCoord).r , 0.0, 1.0);
			col1.g = clamp( texture3D(tex2, TexCoord).g, 0.0, 1.0);
			col1.b = clamp( texture3D(tex2, TexCoord).b, 0.0, 1.0);
			
			if ( ((col1.r + col1.g + col1.b) / 3.0 - thresholdTex2) > 0.0)
			{
				col.rgb = col1.rgb;
			}
		}
		col.a = max(col.a, clamp ((col.r + col.g + col.b)/3.0 - thresholdTex2, 0.0, 1.0));	
	}
	
	if (showTex1)
	{
		if (typeTex1 == 3)
		{
			col1.r = clamp( texture3D(tex1, TexCoord).r, 0.0, 1.0);
			
			if (col1.r - thresholdTex1 > 0.0)
			{
				col.rgb = defaultColorMap( col1.r);
			}
		}	
		if (typeTex1 == 1 || typeTex1 == 2 || typeTex1 == 4)
		{
			col1.r = clamp( texture3D(tex1, TexCoord).r , 0.0, 1.0);
			col1.g = clamp( texture3D(tex1, TexCoord).g, 0.0, 1.0);
			col1.b = clamp( texture3D(tex1, TexCoord).b, 0.0, 1.0);
			
			if ( ((col1.r + col1.g + col1.b) / 3.0 - thresholdTex1) > 0.0)
			{
				col.rgb = col1.rgb;
			}
		}
		col.a = max(col.a, clamp ((col.r + col.g + col.b)/3.0 - thresholdTex1, 0.0, 1.0));
	}
	
	if (showTex0)
	{
		if ( typeTex0 == 3 )
		{
			col1.r = clamp( texture3D(tex0, TexCoord).r, 0.0, 1.0);
			
			if (col1.r - thresholdTex0 > 0.0)
			{
				col.rgb = defaultColorMap( col1.r);
			}
		}	
		if (typeTex0 == 1 || typeTex0 == 2 || typeTex0 == 4)
		{
			col1.r = clamp( texture3D(tex0, TexCoord).r , 0.0, 1.0);
			col1.g = clamp( texture3D(tex0, TexCoord).g, 0.0, 1.0);
			col1.b = clamp( texture3D(tex0, TexCoord).b, 0.0, 1.0);
			
			if ( ((col1.r + col1.g + col1.b) / 3.0 - thresholdTex0) > 0.0)
			{
				col.rgb = col1.rgb;
			}
		}
		col.a = clamp ((col.r + col.g + col.b)/3.0 - thresholdTex0, 0.0, 1.0);
	}
	
	gl_FragColor = col;
}

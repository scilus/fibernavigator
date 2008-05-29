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

void main()
{
	vec4 col = vec4(0);
	vec4 col1;
	
	if (showTex0)
	{
		col = texture3D(tex0, TexCoord);
		col.a = clamp ((col.r + col.g + col.b)/3.0 - thresholdTex0, 0.0, 1.0);	
	}
	
	if (showTex1)
	{
		if (typeTex1 == 3)
		{
			col1.r = clamp( texture3D(tex1, TexCoord).r - thresholdTex1, 0.0, 1.0);
			col1.bg = vec2(0.0, 0.0);
			if (col1.r >0.0)
			{
				col.rgb = col1.rgb;
			}
		}	
		if (typeTex1 == 4)
		{
			col1.r = clamp( texture3D(tex1, TexCoord).r - thresholdTex1, 0.0, 1.0);
			col1.g = clamp( texture3D(tex1, TexCoord).g - thresholdTex1, 0.0, 1.0);
			col1.b = clamp( texture3D(tex1, TexCoord).b - thresholdTex1, 0.0, 1.0);
			
			if (col1.r + col1.g + col1.b > 0.0)
			{
				col.rgb = col1.rgb;
			}
		}
	}
		
	gl_FragColor = col;
}

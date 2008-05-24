uniform sampler3D HeadTexture;
uniform sampler3D OverlayTexture;
uniform sampler3D RGBTexture;
uniform bool showOverlay;
uniform bool showRGB;
varying vec3 TexCoord;

void main()
{
	vec4 col;
	col = texture3D(HeadTexture, TexCoord);
	float value = texture3D(OverlayTexture, TexCoord).r;

	if (showOverlay)
	{
		col.r = clamp(col.r + value, 0.0, 1.0);
		col.g = clamp(col.g - value, 0.0, 1.0);
		col.b = clamp(col.b - value, 0.0, 1.0);
	}

	if (showRGB)
	{
		col = texture3D(RGBTexture, TexCoord);
	}
	
	col.a = texture3D(HeadTexture, TexCoord).a;
	gl_FragColor = col;
}

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

	if (showOverlay)
		col[0] = clamp(col[0] + texture3D(OverlayTexture, TexCoord).r, 0.0, 1.0);

	if (showRGB)
		col = texture3D(RGBTexture, TexCoord);

	col.a = texture3D(HeadTexture, TexCoord).a;
	gl_FragColor = col;
}

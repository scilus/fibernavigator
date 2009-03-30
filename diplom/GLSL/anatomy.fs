#include functions.fs

uniform sampler3D texes[10];
uniform bool show[10];
uniform float threshold[10];
uniform float alpha[10];
uniform int type[10];
uniform int countTextures;

void main()
{
	vec4 col = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = 9 ; i > -1 ; i--)
	{
		if (show[i]) lookupTex(col, type[i], texes[i], threshold[i], alpha[i]);
	}

	col = clamp(col, 0.0, 1.0);

	gl_FragColor = col;
}

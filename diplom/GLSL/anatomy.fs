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
/*
	for (int i = 9 ; i > -1 ; i--)
	{
		if (show[i]) lookupTex(col, type[i], texes[i], threshold[i], alpha[i]);
	}
*/
	if (show[9]) lookupTex(col, type[9], texes[9], threshold[9], alpha[9]);
	if (show[8]) lookupTex(col, type[8], texes[8], threshold[8], alpha[8]);
	if (show[7]) lookupTex(col, type[7], texes[7], threshold[7], alpha[7]);
	if (show[6]) lookupTex(col, type[6], texes[6], threshold[6], alpha[6]);
	if (show[5]) lookupTex(col, type[5], texes[5], threshold[5], alpha[5]);
	if (show[4]) lookupTex(col, type[4], texes[4], threshold[4], alpha[4]);
	if (show[3]) lookupTex(col, type[3], texes[3], threshold[3], alpha[3]);
	if (show[2]) lookupTex(col, type[2], texes[2], threshold[2], alpha[2]);
	if (show[1]) lookupTex(col, type[1], texes[1], threshold[1], alpha[1]);
	if (show[0]) lookupTex(col, type[0], texes[0], threshold[0], alpha[0]);

	col = clamp(col, 0.0, 1.0);

	gl_FragColor = col;
}

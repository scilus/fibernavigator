varying vec4 VaryingTexCoord0;
#include functions.fs

void main()
{
	vec4 col = vec4(0.0, 0.0, 0.0, 0.0);

    if ( type9 > 0 ) lookupTex(col, type9, tex9, threshold9, VaryingTexCoord0.xyz, alpha9);
    if ( type8 > 0 ) lookupTex(col, type8, tex8, threshold8, VaryingTexCoord0.xyz, alpha8);
    if ( type7 > 0 ) lookupTex(col, type7, tex7, threshold7, VaryingTexCoord0.xyz, alpha7);
    if ( type6 > 0 ) lookupTex(col, type6, tex6, threshold6, VaryingTexCoord0.xyz, alpha6);
    if ( type5 > 0 ) lookupTex(col, type5, tex5, threshold5, VaryingTexCoord0.xyz, alpha5);
    if ( type4 > 0 ) lookupTex(col, type4, tex4, threshold4, VaryingTexCoord0.xyz, alpha4);
    if ( type3 > 0 ) lookupTex(col, type3, tex3, threshold3, VaryingTexCoord0.xyz, alpha3);
    if ( type2 > 0 ) lookupTex(col, type2, tex2, threshold2, VaryingTexCoord0.xyz, alpha2);
    if ( type1 > 0 ) lookupTex(col, type1, tex1, threshold1, VaryingTexCoord0.xyz, alpha1);
	if ( type0 > 0 ) lookupTex(col, type0, tex0, threshold0, VaryingTexCoord0.xyz, alpha0);

	col = clamp(col, 0.0, 1.0);

	col.a = sqrt( col.r * col.r + col.g * col.g + col.b * col.b);

	gl_FragColor = col;
}

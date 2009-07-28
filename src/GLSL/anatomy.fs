#include functions.fs

void main()
{
	vec4 col = vec4(0.0, 0.0, 0.0, 0.0);

    if ( show9 ) lookupTex(col, type9, tex9, threshold9, alpha9);
    if ( show8 ) lookupTex(col, type8, tex8, threshold8, alpha8);
    if ( show7 ) lookupTex(col, type7, tex7, threshold7, alpha7);
    if ( show6 ) lookupTex(col, type6, tex6, threshold6, alpha6);
    if ( show5 ) lookupTex(col, type5, tex5, threshold5, alpha5);
    if ( show4 ) lookupTex(col, type4, tex4, threshold4, alpha4);
    if ( show3 ) lookupTex(col, type3, tex3, threshold3, alpha3);
    if ( show2 ) lookupTex(col, type2, tex2, threshold2, alpha2);
    if ( show1 ) lookupTex(col, type1, tex1, threshold1, alpha1);
	if ( show0 ) lookupTex(col, type0, tex0, threshold0, alpha0);
		
	col = clamp(col, 0.0, 1.0);

	gl_FragColor = col;
}

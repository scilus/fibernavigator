#include functions.fs

void main()
{
	float v = gl_TexCoord[0].x;
	vec4 c = vec4(1.0);
	colorMap( c.rgb, v );

	gl_FragColor = c;
}

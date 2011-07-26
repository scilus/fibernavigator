varying vec4 VaryingTexCoord0;

#include functions.fs

void main()
{
	float v = VaryingTexCoord0.x;
	vec4 c = vec4(1.0);
	colorMap( c.rgb, v );

	gl_FragColor = c;
}

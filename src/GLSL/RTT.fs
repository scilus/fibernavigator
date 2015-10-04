#extension GL_ARB_texture_rectangle : enable

uniform int color;
uniform sampler2DRect inSeed, xValues;

void main()
{
	vec4 x = texture2DRect(xValues, gl_TexCoord[0].st);
	vec4 y = texture2DRect(inSeed, gl_TexCoord[0].st);
	vec4 modX = x + y;
	vec4 test = vec4(50.0,50.0,50.0,0.0);
	
	if(color == 1)
		gl_FragColor = modX;
	else
		gl_FragColor = test;
		
}

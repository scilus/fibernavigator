uniform float 		alpha, brightness;
uniform sampler1D 	clut;
uniform int 		colorWithPos;
varying float 		texturePosition;
varying vec3 		vertexPos;

void main()
{	
	// Color with position (x = r, y = g, z = b)
	if( colorWithPos == 1 )
		gl_FragColor.rgb = vertexPos;
	// Color normally
	else
		gl_FragColor = texture1D( clut, texturePosition );
	
	// Adjust the brightness of the fragment.
	gl_FragColor.rgb *= brightness;
	
	// Adjust the alpha of the fragment.
	gl_FragColor.a    = alpha;
}
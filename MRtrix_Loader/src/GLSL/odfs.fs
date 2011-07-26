uniform float 		alpha, brightness;
uniform sampler1D 	clut;
uniform int 		colorWithPos;
varying float 		texturePosition;
varying vec3 		vertexPos;
uniform int			showAxis;

void main()
{	
	if(showAxis == 1)
	{
		gl_FragColor = vec4 (abs(vertexPos),1.0) ;
	}
	else
	{
		// Color with position (x = r, y = g, z = b)
		if( colorWithPos == 1 )
			gl_FragColor.rgb = vertexPos;
		// Color normally
		else
			gl_FragColor = texture1D( clut, texturePosition );
	}
	
	// Adjust the brightness of the fragment.
	gl_FragColor.rgb *= brightness;
	
	// Adjust the alpha of the fragment.
	gl_FragColor.a    = alpha;
}
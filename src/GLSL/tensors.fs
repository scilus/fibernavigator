uniform sampler1D clut;
uniform float     alpha, attenuation, brightness;
uniform int 	  colorWithPos;
varying float     texturePosition;
varying vec3      lightDir, normal;
varying vec3 	  vertexPos;

void main()
{
	vec4 color;
	// Color with position (x = r, y = g, z = b)
	if (colorWithPos==1)		
		color.rgb = vertexPos;
	else
		//get the color from texture
		color = texture1D( clut,texturePosition );
	
	// Compute the dot product between normal and normalized lightdir.
	float NdotL = max( dot( normal, lightDir ), 0.0 );

	// Adjust the color depending on the lighting.
	if( NdotL > 0.0 ) 
		color += attenuation * NdotL;

	// Set the final color of the fragment.
	gl_FragColor = color;
	
	// Adjust the brightness of the fragment.
	gl_FragColor.rgb *= brightness;
	
	// Adjust the alpha of the fragment.
	gl_FragColor.a = alpha;
	
	
		

}
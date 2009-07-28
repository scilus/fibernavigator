#include lighting.fs

uniform int dimX, dimY, dimZ;
uniform float voxX, voxY, voxZ;

uniform int countTextures;
uniform bool useLic;

#include functions.fs

void main() 
{
	/* Normalize the normal. A varying variable CANNOT
	 // be modified by a fragment shader. So a new variable
	 // needs to be created. */
	vec3 n = normal;

	vec4 ambient = vec4(0.0);
	vec4 diffuse = vec4(0.0);
	vec4 specular = vec4(0.0);

	/* In this case the built in uniform gl_MaxLights is used
	 // to denote the number of lights. A better option may be passing
	 // in the number of lights as a uniform or replacing the current
	 // value with a smaller value. */
	calculateLighting(gl_MaxLights, -n, vertex.xyz, gl_FrontMaterial.shininess,
			ambient, diffuse, specular);

	vec4 color = vec4(0.0);

	vec3 v = gl_TexCoord[0].xyz;
	v.x = v.x / (float(dimX) * voxX);
    v.y = v.y / (float(dimY) * voxY);
    v.z = v.z / (float(dimZ) * voxZ);

    if (show9) lookupTexMesh( color, type9, tex9, threshold9, v, alpha9 );
    if (show8) lookupTexMesh( color, type8, tex8, threshold8, v, alpha8 );
    if (show7) lookupTexMesh( color, type7, tex7, threshold7, v, alpha7 );
    if (show6) lookupTexMesh( color, type6, tex6, threshold6, v, alpha6 );
    if (show5) lookupTexMesh( color, type5, tex5, threshold5, v, alpha5 );
    if (show4) lookupTexMesh( color, type4, tex4, threshold4, v, alpha4 );
    if (show3) lookupTexMesh( color, type3, tex3, threshold3, v, alpha3 );
    if (show2) lookupTexMesh( color, type2, tex2, threshold2, v, alpha2 );
    if (show1) lookupTexMesh( color, type1, tex1, threshold1, v, alpha1 );
    if (show0) lookupTexMesh( color, type0, tex0, threshold0, v, alpha0 );
    
    if (color.rgb == vec3(0.0)) discard;

	color.a = 1.0;

	//color = color * 0.8 + (ambient * color / 2.0) + (diffuse * color / 2.0) + (specular * color / 2.0);

	color = clamp(color, 0.0, 1.0);

	if (useLic)
	{
	    // gl_Color.r = LIC texture gray value
        // gl_Color.g = scalar product of triangle normal and input vector
        // gl_Color.b = FA value
        // gl_Color.a = noise texture gray value
	   float fa = clamp( (gl_Color.b - 0.1) * 3., 0., 1. ); 
		
	   float licBlend = (1.0 - gl_Color.g) * fa;
	   vec4 tempColor = vec4(gl_Color.r, gl_Color.r, gl_Color.r, (1.0 - gl_Color.g) );
	   vec4 licColor = clamp( tempColor * 1.8 - vec4(0.4), 0., 1.);
       
       
       float noiseBlend = clamp((gl_Color.g - 0.6),0., 1.) * clamp((fa - 0.2),0., 1.) * 3.;
       vec4 noiseColor = vec4( gl_Color.a );
       
       gl_FragColor = ((noiseColor - vec4(0.5)) * color * noiseBlend) + ((licColor - vec4(0.5)) * color * licBlend) + color;
    }
	else
		gl_FragColor = color;
}

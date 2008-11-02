#include GLSL/lighting.fs

uniform int dimX, dimY, dimZ;

uniform sampler3D texes[10];
uniform bool show[10];
uniform float threshold[10];
uniform int type[10];
uniform int countTextures;

vec3 defaultColorMap( float value )
{
    value *= 5.0;
	vec3 color;

	if( value < 0.0 )
		color = vec3( 0.0, 0.0, 0.0 );
    else if( value < 1.0 )
		color = vec3( 0.0, value, 1.0 );
	else if( value < 2.0 )
		color = vec3( 0.0, 1.0, 2.0-value );
    else if( value < 3.0 )
		color =  vec3( value-2.0, 1.0, 0.0 );
    else if( value < 4.0 )
		color = vec3( 1.0, 4.0-value, 0.0 );
    else if( value <= 5.0 )
		color = vec3( 1.0, 0.0, value-4.0 );
    else
		color =  vec3( 1.0, 0.0, 1.0 );
	return color;
}

void lookupTex(inout vec4 color, in int type, in sampler3D tex, in float threshold, in vec3 v)
{
	vec3 col1;
	if (type == 3)
	{
		col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);

		if (col1.r - threshold > 0.0)
		{
			color.rgb = defaultColorMap( col1.r);
		}
	}
	if (type == 1 || type == 2 || type == 4)
	{
		col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);
		col1.g = clamp( texture3D(tex, v).g, 0.0, 1.0);
		col1.b = clamp( texture3D(tex, v).b, 0.0, 1.0);

		if ( ((col1.r + col1.g + col1.b) / 3.0 - threshold) > 0.0)
		{
			color.rgb = col1.rgb;
		}
	}
}


void main()
{
	/* Normalize the normal. A varying variable CANNOT
    // be modified by a fragment shader. So a new variable
    // needs to be created. */
    vec3 n = normal;

    vec4 ambient  = vec4(0.0);
    vec4 diffuse  = vec4(0.0);
    vec4 specular = vec4(0.0);

    /* In this case the built in uniform gl_MaxLights is used
    // to denote the number of lights. A better option may be passing
    // in the number of lights as a uniform or replacing the current
    // value with a smaller value. */
    calculateLighting(gl_MaxLights, -n, vertex.xyz, gl_FrontMaterial.shininess,
                      ambient, diffuse, specular);

   vec4 color = vec4(0.0);

	vec3 v = gl_TexCoord[0].xyz;
	v.x = (v.x) / float(dimX);
	v.y = (v.y) / float(dimY);
	v.z = (v.z) / float(dimZ);

	for (int i = 9 ; i > -1 ; i--)
	{
		if (show[i]) lookupTex(color, type[i], texes[i], threshold[i], v);
	}

	if (color.rgb == vec3(0.0)) discard;

	color.a = 1.0;

	color = color * 0.8 + (ambient * color / 2.0) + (diffuse * color /2.0) + (specular * color / 2.0);

	color = clamp(color, 0.0, 1.0);

    gl_FragColor = color;
}

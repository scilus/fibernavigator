#include GLSL/lighting.fs

varying vec3 color1;
varying vec3 TexCoord;

uniform int dimX, dimY, dimZ;
uniform sampler3D texes[10];
uniform bool show[10];
uniform float threshold[10];
uniform int type[10];
uniform int countTextures;

void lookupTex( in sampler3D tex, in float threshold, in vec3 v)
{
	vec3 col1;
	col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);

	if ( col1.r  < threshold )
	{
		discard;
	}

}

void main()
{
	vec3 v = TexCoord;
	v.x = (v.x + dimX / 2) / (float) dimX;
	v.y = (v.y + dimY / 2) / (float) dimY;
	v.z = (v.z + dimZ / 2) / (float) dimZ;

	for (int i = 9; i > -1; i--) {
		if (type[i] == 3 && show[i])
			lookupTex(texes[i], threshold[i], v);
	}

	vec4 color = vec4(0.0);

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
	calculateLighting(gl_MaxLights, -n, vertex, gl_FrontMaterial.shininess,
					  ambient, diffuse, specular);

	vec4 tmpColor = vec4(color1, 1.0);

   //tmpColor = gl_FrontLightModelProduct.sceneColor;

	color = tmpColor + (ambient * tmpColor / 2.0) + (diffuse * tmpColor /2.0) + (specular * tmpColor / 2.0);

   //color =   tmpColor + (ambient  * gl_FrontMaterial.ambient) + (diffuse  * gl_FrontMaterial.diffuse/2.0);

   color = clamp(color, 0.0, 1.0);

   gl_FragColor = color;
}

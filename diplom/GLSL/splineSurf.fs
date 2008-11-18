#include lighting.fs

uniform int dimX, dimY, dimZ;

uniform sampler3D texes[10];
uniform bool show[10];
uniform float threshold[10];
uniform float alpha[10];
uniform int type[10];
uniform int countTextures;
uniform bool useLic;

#include functions.fs

void main() {
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
	v.x = (v.x) / float(dimX);
	v.y = (v.y) / float(dimY);
	v.z = (v.z) / float(dimZ);

	for (int i = 9; i > -1; i--) {
		if (show[i]) lookupTexMesh(color, type[i], texes[i], threshold[i], v, alpha[i]);
	}

	if (color.rgb == vec3(0.0)) discard;

	color.a = 1.0;

	color = color * 0.8 + (ambient * color / 2.0) + (diffuse * color / 2.0)
			+ (specular * color / 2.0);

	color = clamp(color, 0.0, 1.0);

	if (useLic)
		gl_FragColor = (0.7  * gl_Color) + (0.3 * color);
	else
		gl_FragColor = color;
	//gl_FragColor = gl_Color;
}

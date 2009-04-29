#include lighting.fs

uniform int dimX, dimY, dimZ;
uniform float voxX, voxY, voxZ;

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
	v.x = v.x / (float(dimX) * voxX);
    v.y = v.y / (float(dimY) * voxY);
    v.z = v.z / (float(dimZ) * voxZ);

	for (int i = 9; i > -1; i--) {
		if (show[i]) lookupTexMesh(color, type[i], texes[i], threshold[i], v, alpha[i]);
	}

	if (color.rgb == vec3(0.0)) discard;

	color.a = 1.0;

	color = color * 0.8 + (ambient * color / 2.0) + (diffuse * color / 2.0)
			+ (specular * color / 2.0);

	color = clamp(color, 0.0, 1.0);

	if (useLic)
//		gl_FragColor = (0.7  * gl_Color) + (0.3 * color);
		gl_FragColor = clamp(((gl_Color + vec4(0.3)) * color ), 0.0, 1.0);
	else
		gl_FragColor = color;
	//gl_FragColor = gl_Color;
}

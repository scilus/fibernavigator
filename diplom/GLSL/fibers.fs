#include GLSL/lighting.fs

varying vec3 TexCoord;
varying vec4 color1;

uniform int dimX, dimY, dimZ;
uniform sampler3D tex;
uniform bool show;
uniform float threshold;
uniform int type;
uniform bool useTex;

void lookupTex() {
	vec3 v = TexCoord;
	v.x = (v.x + dimX / 2) / (float) dimX;
	v.y = (v.y + dimY / 2) / (float) dimY;
	v.z = (v.z + dimZ / 2) / (float) dimZ;

	vec3 col1;
	col1.r = clamp(texture3D(tex, v).r, 0.0, 1.0);

	if (col1.r < threshold) {
		discard;
	}

}

void main() {
	if (type == 3 && useTex)
		lookupTex();

	vec4 color = vec4(0.0);

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
	calculateLighting(gl_MaxLights, -n, vertex, gl_FrontMaterial.shininess,
			ambient, diffuse, specular);

	 color = color1 + (ambient  * gl_FrontMaterial.ambient)
				    + (diffuse  * gl_FrontMaterial.diffuse / 2.0)
					+ (specular * gl_FrontMaterial.specular /2.0);

	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color;
}

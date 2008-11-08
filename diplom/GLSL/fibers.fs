#include lighting.fs

uniform int dimX, dimY, dimZ;
uniform sampler3D tex;
uniform bool show;
uniform float threshold;
uniform int type;
uniform bool useTex;
uniform bool lightOn;

varying vec4 myColor;

void lookupTex() {
	vec3 v = gl_TexCoord[0].xyz;
	v.x = (v.x) / float( dimX);
	v.y = (v.y) / float( dimY);
	v.z = (v.z) / float( dimZ);

	vec3 col1;
	col1.r = clamp(texture3D(tex, v).r, 0.0, 1.0);

	if (col1.r < threshold) {
		discard;
	}

}

void main() {
	if (type == 3 && useTex)
		lookupTex();

	if (lightOn)
	{
		vec3 n = normal.xyz;

		vec4 ambient = vec4(0.0);
		vec4 diffuse = vec4(0.0);
		vec4 specular = vec4(0.0);

		/* In this case the built in uniform gl_MaxLights is used
		 // to denote the number of lights. A better option may be passing
		 // in the number of lights as a uniform or replacing the current
		 // value with a smaller value. */
		calculateLighting(gl_MaxLights, -n, vertex.xyz, gl_FrontMaterial.shininess,
				ambient, diffuse, specular);

		gl_FragColor = myColor + (ambient * myColor) + (diffuse * myColor)
						+ (specular * myColor);

	}
	else
		gl_FragColor = myColor;
}

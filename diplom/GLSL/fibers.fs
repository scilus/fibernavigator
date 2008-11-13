varying vec3 normal;
varying vec3 position;

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
	v.x = (v.x) / float(dimX);
	v.y = (v.y) / float(dimY);
	v.z = (v.z) / float(dimZ);

	vec3 col1;
	col1.r = clamp(texture3D(tex, v).r, 0.0, 1.0);

	if (col1.r < threshold) {
		discard;
	}

}

void main() {
	if (type == 3 && useTex)
		lookupTex();

	if (lightOn) {
		// normalize the vertex normal and the view vector
		vec3 norm = abs(normal);
		vec3 view = normalize(-position);

		vec4 ambient = vec4(0.0); //gl_FrontLightModelProduct.sceneColor;
		vec4 diffuse = vec4(0.0);
		vec4 specular = vec4(0.0);

		// determine the light and light reflection vectors
		//vec3 light = normalize(gl_LightSource[0].position.xyz - position);
		vec3 light = (gl_ModelViewMatrix * vec4(0., 0., -1., 0.)).xyz;
		//vec3 light = vec3(0.0, 0.0, -1.0);
		vec3 reflected = -reflect(light, norm);

		// add the current light's ambient value
		ambient += gl_FrontLightProduct[0].ambient;

		// calculate and add the current light's diffuse value
		vec4 calculatedDiffuse = vec4(max(dot(norm, light), 0.0));
		diffuse += gl_FrontLightProduct[0].diffuse * calculatedDiffuse;

		// calculate and add the current light's specular value
		vec4 calculatedSpecular = vec4(pow(max(dot(reflected, view), 0.0), 0.3
				* gl_FrontMaterial.shininess));
		specular += clamp(gl_FrontLightProduct[0].specular
				* calculatedSpecular, 0.0, 1.0);

		gl_FragColor = myColor + diffuse;// + specular;

	} else
		gl_FragColor = myColor;
}

#include GLSL/lighting.fs

varying vec3 color1;

void main()
{
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

   /* tmpColor = gl_FrontLightModelProduct.sceneColor; */

   color =   tmpColor +
			  (ambient  * gl_FrontMaterial.ambient) +
			  (diffuse  * gl_FrontMaterial.diffuse/2.0);

   color = clamp(color, 0.0, 1.0);

   gl_FragColor = color;
}

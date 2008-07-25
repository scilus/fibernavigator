varying vec3 color1;
varying vec3 color2;
uniform bool useNormals;

uniform float cam[3];

void main()
{
	vec4 color = vec4(0.0);

	/* Normalize the normal. A varying variable CANNOT
	// be modified by a fragment shader. So a new variable
	// needs to be created. */

	vec3 c = vec3(cam[0], cam[1], cam[2]);
	vec3 c1 = normalize(gl_NormalMatrix * c);
	vec3 c2 = cross(normal,c);

	vec3 n = c2;

	vec4 ambient  = vec4(0.0);
	vec4 diffuse  = vec4(0.0);
	vec4 specular = vec4(0.0);

	/* In this case the built in uniform gl_MaxLights is used
	// to denote the number of lights. A better option may be passing
	// in the number of lights as a uniform or replacing the current
	// value with a smaller value. */
	calculateLighting(gl_MaxLights, -n, vertex, gl_FrontMaterial.shininess,
					  ambient, diffuse, specular);

   vec4 tmpColor = vec4(0.0);


   if (!useNormals)
		tmpColor = vec4(color1, 1.0);
   else
		tmpColor = vec4(color2, 1.0);

   /* tmpColor = gl_FrontLightModelProduct.sceneColor; */


   color =   tmpColor +
			  (ambient  * gl_FrontMaterial.ambient) +
			  (diffuse  * gl_FrontMaterial.diffuse/2.0);

   color = clamp(color, 0.0, 1.0);

   gl_FragColor = color;
}

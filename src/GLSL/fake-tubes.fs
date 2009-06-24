varying vec3 tangentR3; // Tangent vector in world space
varying float s_param; // s parameter of texture [-1..1]
varying float tangent_dot_view;
varying vec4 myColor;
uniform bool globalColor;

/*
 * simple fragment shader that does rendering of tubes with diffuse illumination
 */
void main() {
	vec3 color;
	if (globalColor)
		color = abs(normalize(myColor.rgb));
	else
		color = abs(normalize(tangentR3));
	vec3 view = vec3(0., 0., -1.);
	float view_dot_normal = sqrt(1. - s_param * s_param) + .1;

	float s = (s_param + 1.) * 0.5; //< normalize in [0..1]
	gl_FragColor.rgb = clamp(view_dot_normal * (color + 0.15 * pow(
			view_dot_normal, 10.) * pow(tangent_dot_view, 10.) * vec3(1., 1.,
			1.)), 0., 1.); //< set the color of this fragment (i.e. pixel)
	gl_FragColor.a = myColor.a;
}


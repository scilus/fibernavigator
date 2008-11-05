//varying vec3 tangent;
varying float tangent_dot_view;
varying vec3 tangentR3;
varying float s_param;

void main() {
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex; //< make clipping planes work

	vec3 tangent;

	float thickness = 0.01;

	tangentR3 = gl_Normal;
	//tangent = gl_NormalMatrix * gl_Normal; //< transform our tangent vector
	tangent = (gl_ModelViewProjectionMatrix * vec4(gl_Normal,0.)).xyz; //< transform our tangent vector
	s_param = gl_MultiTexCoord0.x; //< store texture coordinate for shader

	vec3 eyeVec = vec3(gl_ModelViewMatrix * gl_Vertex);
	vec3 viewVec = normalize(-eyeVec);

	vec3 offsetNN = cross(vec3(0., 0., -1.), normalize(tangent.xyz));
	vec3 offset = normalize(offsetNN);
	tangent_dot_view = length(offsetNN);

	vec4 pos = ftransform(); //< transform position to eye space

	// compute something like a perspective correction factor and zoom factor
	//vec4 p1 = vec4(offset.x*thickness,offset.y*thickness,-pos.z, 1.);
	//vec4 p2 = gl_ProjectionMatrix * p1;
	//float scale = length(p2.xy);


	pos.xyz += offset * (s_param * thickness); //< add offset in y-direction (eye-space)


	gl_Position = pos; //< store final position
}

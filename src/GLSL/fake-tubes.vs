uniform float dimX, dimY;

varying float tangent_dot_view;
varying vec3 tangentR3;
varying float s_param;
varying vec4 myColor;
uniform float thickness;

void main()
{
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex; //< make clipping planes work

	vec3 tangent;

	tangentR3 = gl_Normal;
	tangent = (gl_ModelViewProjectionMatrix * vec4(gl_Normal,0.)).xyz; //< transform our tangent vector
	s_param = gl_MultiTexCoord0.x; //< store texture coordinate for shader

	vec3 v = (gl_ModelViewMatrix * vec4(0., 0., -1., 0.)).xyz;
	vec3 offsetNN = cross(v, normalize(tangent.xyz));
	vec3 offset = normalize(offsetNN);
	tangent_dot_view = length(offsetNN);

	vec4 pos = ftransform(); //< transform position to eye space

	offset.x *= thickness / dimX;
	offset.y *= thickness / dimY;

	pos.xyz += offset * (s_param); //< add offset in y-direction (eye-space)

	myColor = gl_Color;

	gl_Position = pos; //< store final position
}

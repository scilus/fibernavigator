void main()
{
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;

	float scale = 0.5;
	float s_param = gl_MultiTexCoord0.x;
	vec4 pos = gl_Vertex ;

	vec3 A = gl_Color.rgb;
	vec3 B = gl_Normal;

	vec3 offset = cross(B, (cross(A,B))) * s_param * scale;
	pos.xyz += offset;
	// a small offset so they don't disappear in the surface
	pos.x -= 0.1;
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}

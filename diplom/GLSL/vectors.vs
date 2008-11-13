void main()
{
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;

	float scale = 1.0;
	float s_param = gl_MultiTexCoord0.x;
	vec3 offset = s_param * scale * normalize(gl_Color.rgb);
	vec4 pos = gl_Vertex ;
	pos.xyz += offset;

	gl_Position = gl_ModelViewProjectionMatrix * pos;
}

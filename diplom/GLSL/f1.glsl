uniform float blendThreshold;

uniform sampler3D tex0;
uniform sampler3D tex1;
uniform sampler3D tex2;
uniform sampler3D tex3;
uniform sampler3D tex4;
uniform sampler3D tex5;
uniform sampler3D tex6;
uniform sampler3D tex7;
uniform sampler3D tex8;
uniform sampler3D tex9;

varying vec3 TexCoord;

void main()
{
	vec4 col;
	col = texture3D(tex0, TexCoord);
	col.a = clamp (texture3D(tex0, TexCoord).r - blendThreshold, 0.0, 1.0);
		
	gl_FragColor = col;
}

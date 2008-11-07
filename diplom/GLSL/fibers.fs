uniform int dimX, dimY, dimZ;
uniform sampler3D tex;
uniform bool show;
uniform float threshold;
uniform int type;
uniform bool useTex;
uniform bool lightOn;

varying vec4 myColor;

varying vec3 N, L;

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
		vec3 NN = normalize(N);
		vec3 NL = normalize(L);
		vec3 NH = normalize(NL + vec3(0.0, 0.0, 1.0));
		float NdotL = max (0.0, dot(NN, NL));

		gl_FragColor.rgb = (myColor.rgb * NdotL) + myColor.rgb;
		if (NdotL > 0.0)
			gl_FragColor.rgb += pow(max(0.0, dot(NN, NH)), 128.0)/ 4.0;

	}
	else
		gl_FragColor  = myColor;

	gl_FragColor.a = myColor.a;
}

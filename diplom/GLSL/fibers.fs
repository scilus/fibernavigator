#include functions.fs

uniform int dimX, dimY, dimZ;
uniform sampler3D tex;
uniform bool show;
uniform float threshold;
uniform int type;
uniform bool useTex;
uniform bool lightOn;

varying vec4 myColor;

float lookupTex() {
	vec3 v = gl_TexCoord[0].xyz;
	v.x = (v.x) / float(dimX);
	v.y = (v.y) / float(dimY);
	v.z = (v.z) / float(dimZ);

	vec3 col1;
	col1.r = clamp(texture3D(tex, v).r, 0.0, 1.0);

	if (col1.r < threshold) {
		discard;
	}
	else
	   return col1.r;
}

void main() {
	vec4 cooloor = vec4(1.0);
	if (type == 3 && useTex)
		if ( useColorMap == 1 )
			cooloor.rgb  = colorMap1( lookupTex() );
		else if ( useColorMap == 2 )
			cooloor.rgb  = colorMap2( lookupTex() );
		else if ( useColorMap == 3 )
			cooloor.rgb  = colorMap3( lookupTex() );
		else if ( useColorMap == 4 )
			cooloor.rgb  = colorMap4( lookupTex() );
		else
			cooloor.rgb = defaultColorMap( lookupTex() );
	gl_FragColor = cooloor;
}

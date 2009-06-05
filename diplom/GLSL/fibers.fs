#include functions.fs

uniform int dimX, dimY, dimZ;
uniform float voxX, voxY, voxZ;
uniform sampler3D tex;
uniform float threshold;
uniform int type;
uniform bool useTex;
uniform bool useOverlay;

varying vec4 myColor;

float lookupTex() 
{
	vec3 v = gl_TexCoord[0].xyz;
	v.x = v.x / (float(dimX) * voxX);
    v.y = v.y / (float(dimY) * voxY);
    v.z = v.z / (float(dimZ) * voxZ);

	vec3 col1;
	col1.r = clamp(texture3D(tex, v).r, 0.0, 1.0);

	if (col1.r < threshold) {
		discard;
	}
	else
	   return col1.r;
}

void main() 
{
	vec4 cooloor = vec4(1.0);
	float value = lookupTex();
	float newVal;
	if (threshold < 1.0)
		newVal = (value - threshold) / (1.0 - threshold);
	else
		newVal = 1.0;
	
	if (type == 3 && useTex)
		if ( useColorMap == 1 )
			cooloor.rgb  = colorMap1( newVal );
		else if ( useColorMap == 2 )
			cooloor.rgb  = colorMap2( newVal );
		else if ( useColorMap == 3 )
			cooloor.rgb  = colorMap3( newVal );
		else if ( useColorMap == 4 )
			cooloor.rgb  = colorMap4( newVal );
		else
			cooloor.rgb = defaultColorMap( newVal );

 /*  cooloor.a = 1.-(1.-newVal)*(1.-newVal);
     cooloor.a = newVal*newVal;*/

	cooloor.a = newVal * myColor.a;
	if ( useOverlay )
	   cooloor.rgb = myColor.rgb;
	gl_FragColor = cooloor;
}

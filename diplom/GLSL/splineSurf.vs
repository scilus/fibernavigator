#include GLSL/lighting.vs

varying vec3 TexCoord;

uniform sampler3D texes[10];
uniform int type[10];
uniform int dimX, dimY, dimZ;

void lookupTex(inout float greyVal, in int type, in sampler3D tex, in vec3 v)
{
	if (type == 1)
	{
		greyVal = clamp( texture3D(tex, v).r, 0.0, 1.0);
	}
}

void main()
{
	prepareLight();

	float greyVal = 0.0;

	vec3 v = gl_Vertex;
	v.x = (v.x + dimX/2) / (float)dimX;
	v.y = (v.y + dimY/2) / (float)dimY;
	v.z = (v.z + dimZ/2) / (float)dimZ;

	for (int i = 9 ; i > -1 ; i--)
	{
		if (type[i] == 1)
		{
			greyVal = clamp( texture3D(texes[i], v).r, 0.0, 1.0);
		}
	}
	vec3 offset = (greyVal - 0.5) * gl_Normal;
	vec4 newVert = gl_Vertex;
	newVert.xyz  += 5 * offset;

	TexCoord = newVert;
    gl_Position = gl_ModelViewProjectionMatrix * newVert;
}

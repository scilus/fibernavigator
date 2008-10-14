#include GLSL/lighting.vs

varying vec3 TexCoord;

uniform sampler3D texes[10];
uniform int type[10];
uniform int dimX, dimY, dimZ;

void main()
{
	prepareLight();

	float greyVal = 0.5;

	vec4 newVert = gl_Vertex;

	for (int j = 0 ; j < 3 ; ++j)
	{
		vec3 v = newVert.xyz;
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

		newVert.xyz  += 4 * offset;
	}

	TexCoord = newVert;
    gl_Position = gl_ModelViewProjectionMatrix * newVert;
}

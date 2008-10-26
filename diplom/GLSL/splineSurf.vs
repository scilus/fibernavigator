#include GLSL/lighting.vs

varying vec3 TexCoord;
varying vec4 myColor;

uniform sampler3D texes[10];
uniform int type[10];
uniform int dimX, dimY, dimZ;

void main()
{
	prepareLight();

	float greyVal = 0.5;

	for (int j = 0 ; j < 6 ; ++j)
	{
		vec3 v = gl_Vertex.xyz;
		v.x = (v.x) / (float)dimX;
		v.y = (v.y) / (float)dimY;
		v.z = (v.z) / (float)dimZ;

		for (int i = 9 ; i > -1 ; i--)
		{
			if (type[i] == 1)
			{
				greyVal = clamp( texture3D(texes[i], v).r, 0.0, 1.0);
			}
		}
		vec3 offset = (greyVal - 0.5) * gl_Normal;

		gl_Vertex.xyz  += 2 * offset;
	}

	TexCoord = gl_Vertex;
	myColor = gl_Color;
	gl_Position = ftransform();
}

#include lighting.vs

varying vec4 VaryingTexCoord0;
uniform sampler3D texes[10];
uniform int type[10];
uniform int dimX, dimY, dimZ;
uniform bool useTex;

void main() {
	prepareLight();

	// pass the color to the fragment shader
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;

	if (useTex)
	{
		float greyVal = 0.5;
		vec4 myVert = gl_Vertex;

		for (int j = 0; j < 5; ++j) {
			vec3 v = myVert.xyz;
			v.x = (v.x) / float(dimX);
			v.y = (v.y) / float(dimY);
			v.z = (v.z) / float(dimZ);

			for (int i = 9; i > -1; i--) {
				if (type[i] == 1) {
					greyVal = clamp(texture3D(texes[i], v).r, 0.0, 1.0);
				}
			}
			vec3 offset = (greyVal - 0.5) * gl_Normal;

			// FIXME: we cannot modify gl_Vertex, so we need to copy it!
			// but if we copy it, we cannot use ftransform, so we have to fix the code below!
			myVert.xyz += 2.0 * offset;
		}

		VaryingTexCoord0 = myVert;
		gl_Position = gl_ModelViewProjectionMatrix * myVert;
	}

	else
	{
	    VaryingTexCoord0 = gl_Vertex;
		gl_Position = ftransform();
	}
}

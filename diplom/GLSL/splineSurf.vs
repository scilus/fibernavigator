#include GLSL/lighting.vs

uniform sampler3D texes[10];
uniform int type[10];
uniform int dimX, dimY, dimZ;
uniform bool useTex;

void main() {
	prepareLight();

	if (useTex) {
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

		gl_TexCoord[0].xyz = myVert.xyz;
		gl_Position = gl_ModelViewProjectionMatrix * myVert;
	}

	else {
		gl_TexCoord[0].xyz = gl_Vertex.xyz;
		gl_Position = ftransform();
	}
}

///////////////////////////////////////////////////////////////////////////////
 //
 ///// /////  ////       Project  :   FAnToM
 //   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
 //   //  // /////       File     :   $RCSfile: $
 //   //  // //          Language :   C++
 //    /////  ////       Date     :   $Date: $
 //             Author   :   $Author: ebaum $
 //////////              Revision :   $Revision: 8139 $


// x direction minimum
uniform float minX;

// x direction maximum
uniform float maxX;

// y direction minimum
uniform float minY;

// y direction maximum
uniform float maxY;

// z direction minimum
uniform float minZ;

// z direction maximum
uniform float maxZ;

varying vec3 TexCoord;

uniform sampler3D texes[10];
uniform int type[10];
uniform int dimX, dimY, dimZ;

/////////////////////////////////////////////////////////////////////////////////////////////
 // Transform -- vertex shader -- main
 //
 // Gets called for every vertex and uses the color as tensor and transforms it into image
 // space. All interpolation is done by GPU.
 /////////////////////////////////////////////////////////////////////////////////////////////
void main() {

	/*
	float greyVal = 0.5;

	for (int j = 0; j < 6; ++j) {
		vec3 v = gl_Vertex.xyz;
		v.x = (v.x) / float( dimX);
		v.y = (v.y) / float( dimY);
		v.z = (v.z) / float( dimZ);

		for (int i = 9; i > -1; i--) {
			if (type[i] == 1) {
				greyVal = clamp(texture3D(texes[i], v).r, 0.0, 1.0);
			}
		}
		vec3 offset = (greyVal - 0.5) * gl_Normal;

		gl_Vertex.xyz += 2 * offset;
	}
*/
	TexCoord = gl_Vertex.xyz;

	vec4 tensor = gl_Color;

	/////////////////////////////////////////////////////////////////////////////////////////////
	 // UNSCALE TENSOR
	 //
	 // Unscale tensor data using given min max information
	 //////////////////////////////////////////////////////////////////////////////////////////////

	tensor.x = (tensor.r * (maxX - minX)) + minX;
	tensor.y = (tensor.g * (maxY - minY)) + minY;
	tensor.z = (tensor.b * (maxZ - minZ)) + minZ;
	tensor.w = 1.0; // w component

	/////////////////////////////////////////////////////////////////////////////////////////////
	 // TRANSFORM INTO IMAGE SPACE
	 //
	 // Use gl_ModelViewProjectionMatrix to transform the tensor to image space
	 /////////////////////////////////////////////////////////////////////////////////////////////

	// project
	vec4 base = gl_Vertex;
	vec4 diff = gl_Vertex + tensor;
	diff.w = 1.0;

	vec4 diffP = gl_ModelViewProjectionMatrix * diff;
	vec4 baseP = gl_ModelViewProjectionMatrix * base;

	// scale by w
	diffP.x /= diffP.w;
	diffP.y /= diffP.w;
	diffP.z /= diffP.w;

	baseP.x /= baseP.w;
	baseP.y /= baseP.w;
	baseP.z /= baseP.w;

	tensor = diffP - baseP;

	// map to [0,1]
	tensor.x = tensor.x * 0.5 + 0.5;
	tensor.y = tensor.y * 0.5 + 0.5;
	tensor.z = tensor.z * 0.5 + 0.5;

	// pass the color to the fragment shader
	gl_FrontColor = tensor;
	gl_BackColor = tensor;

	// allow usage of clipping planes
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

	// transform position
	gl_Position = ftransform();
}

//////////////////////////////////////////////////////////////////////////////
 //
 ///// /////  ////       Project  :   FAnToM
 //   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
 //   //  // /////       File     :   $RCSfile: $
 //   //  // //          Language :   C++
 //    /////  ////       Date     :   $Date: $
 //             Author   :   $Author: ebaum $
 //////////              Revision :   $Revision: 8207 $



// the texture unit to use
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

// the size of the texture
uniform float textureSizeW;
uniform float textureSizeH;

//! the size of the textureArea
uniform float textureAreaSizeW;
uniform float textureAreaSizeH;

// noise blending factor
uniform float noiseBlend;

// scaler for the tensors
uniform float tensorAdvectionScale;

// animation flag
uniform float advectionAnimation;

// time counter
uniform int fantom_time;

/////////////////////////////////////////////////////////////////////////////////////////////
 // Advection -- fragment shader -- sharpen
 //
 // Simple sharpen filter. Returns (0,0,0,1) if noise is below threshold, returns
 // (blended) noise else.
 ////////////////////////////////////////////////////////////////////////////////////////////
vec4 sharpen(vec4 noise) {
	float e;
	float c = 0.5; // threshold value
	float cinv = 1.0; // blending factor, in case the noise value is larger than threshold

	if (noise.r > c)
		e = cinv * noise.r;
	else
		e = 0.0;

	return vec4(e, e, e, 1.0);
}

/////////////////////////////////////////////////////////////////////////////////////////////
 // Advection -- fragment shader -- nnn
 //
 // Noise modification. For timebased animation.
 ////////////////////////////////////////////////////////////////////////////////////////////
vec4 nnn(vec4 noise) {
	//float ts = advectionAnimation * (fantom_time ) * 3.14 * 0.001;

	//return noise;
	//  return sin(noise*3.14*2. + ts);

	//return sharpen(sin(noise * 3.14 * 2.0 + ts));
	return sharpen(sin(noise * 3.14 * 2.0 ));

}

/////////////////////////////////////////////////////////////////////////////////////////////
 // Advection -- fragment shader -- main
 //
 // Gets called for every fragment and uses color of first texture as tensors to advect the
 // second (noise) texture
 ////////////////////////////////////////////////////////////////////////////////////////////
void main() {
	vec4 tensor = texture2D(tex0, gl_TexCoord[0].st);
	vec4 noise = nnn(texture2D(tex1, gl_TexCoord[1].st));
	vec4 edge = texture2D(tex2, gl_TexCoord[2].st);

	// unscale
	tensor = (tensor - vec4(0.5, 0.5, 0.5, 0.0)) * 2.0;
	tensor = normalize(tensor); // this prevents some very fast particles to cause some ugly artefacts

	/////////////////////////////////////////////////////////////////////////////////////////////
	 // ADVECT
	 //
	 // Using the tensor to advect
	 /////////////////////////////////////////////////////////////////////////////////////////////

	// calculate position
	vec2 advectedPosition1 = gl_TexCoord[3].st - (tensorAdvectionScale * vec2(
			tensor.x / textureSizeW, tensor.y / textureSizeH));
	vec4 advectedNoise1 = texture2D(tex3, advectedPosition1);

	vec2 advectedPosition2 = gl_TexCoord[3].st + (tensorAdvectionScale  * vec2(
			tensor.x / textureSizeW, tensor.y / textureSizeH));
	vec4 advectedNoise2 = texture2D(tex3, advectedPosition2);

	vec4 advectedNoise3 = (advectedNoise1 + advectedNoise2) / 2.0;

	gl_FragColor = (advectedNoise3 * (1.0 - noiseBlend)) + (noise * noiseBlend);

}


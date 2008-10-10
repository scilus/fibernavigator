/*//////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 8039 $


// the texture unit to use */
uniform sampler2D tex;

/* the size of the texture */
uniform float textureSizeW;
uniform float textureSizeH;

/*////////////////////////////////////////////////////////////////////////////////////////////
// Edge Detection -- fragment shader -- main
//
// Gets called for every fragment and uses surrounding texels for edge detection
/////////////////////////////////////////////////////////////////////////////////////////////*/
void main()
{
  /*////////////////////////////////////////////////////////////////////////////////////////////
  // GETTING TEXELS
  //
  // Get surrounding texels; needed for ALL filters
  ////////////////////////////////////////////////////////////////////////////////////////////*/


  /* get data of surrounding textels */
  float offsetW = 1.0 / textureSizeW;
  float offsetH = 1.0 / textureSizeH;
  vec2 texCoord = gl_TexCoord[0].st;
  vec4 c  = texture2D(tex, texCoord);
  vec4 bl = texture2D(tex, texCoord + vec2(-offsetW, -offsetH));
  vec4 l  = texture2D(tex, texCoord + vec2(-offsetW,     0.0));
  vec4 tl = texture2D(tex, texCoord + vec2(-offsetW,  offsetH));
  vec4 t  = texture2D(tex, texCoord + vec2(    0.0,   offsetH));
  vec4 tr = texture2D(tex, texCoord + vec2( offsetW,  offsetH));
  vec4 r  = texture2D(tex, texCoord + vec2( offsetW,     0.0));
  vec4 br = texture2D(tex, texCoord + vec2( offsetW,  offsetH));
  vec4 b  = texture2D(tex, texCoord + vec2(    0.0,  -offsetH));



  /*////////////////////////////////////////////////////////////////////////////////////////////
  // LAPLACE
  //
  // apply a standart laplace filter kernel
  ////////////////////////////////////////////////////////////////////////////////////////////*/

  /* laplace filter kernel */
  gl_FragColor=abs(1.0/1.0 * (
    0.0 *tl +  1.0 *t + 0.0 *tr +
    1.0 *l  + -4.0 *c + 1.0 *r  +
    0.0 *bl +  1.0 *b + 0.0 *br
  ));


  /*////////////////////////////////////////////////////////////////////////////////////////////
  // LAPLACE OF GAUSSIAN
  //
  // Get additional texture data and apply a combined (laplace+gauss) filter kernel
  ////////////////////////////////////////////////////////////////////////////////////////////*/

/*
  // get data of additional sourrounding texels
  vec4 _t   = texture2D(tex, texCoord + vec2(         0.0,  2.0*offset));
  vec4 _b   = texture2D(tex, texCoord + vec2(         0.0, -2.0*offset));

  vec4 _l   = texture2D(tex, texCoord + vec2( -2.0*offset,         0.0));
  vec4 _r   = texture2D(tex, texCoord + vec2(  2.0*offset,         0.0));

  vec4 _bl2 = texture2D(tex, texCoord + vec2(     -offset, -2.0*offset));
  vec4 _br2 = texture2D(tex, texCoord + vec2(      offset, -2.0*offset));

  vec4 _bl1 = texture2D(tex, texCoord + vec2( -2.0*offset, -2.0*offset));
  vec4 _br1 = texture2D(tex, texCoord + vec2(  2.0*offset, -2.0*offset));

  vec4 _bl3 = texture2D(tex, texCoord + vec2( -2.0*offset,     -offset));
  vec4 _br3 = texture2D(tex, texCoord + vec2(  2.0*offset,     -offset));


  vec4 _tl2 = texture2D(tex, texCoord + vec2(     -offset,  2.0*offset));
  vec4 _tr2 = texture2D(tex, texCoord + vec2(      offset,  2.0*offset));

  vec4 _tl1 = texture2D(tex, texCoord + vec2( -2.0*offset,  2.0*offset));
  vec4 _tr1 = texture2D(tex, texCoord + vec2(  2.0*offset,  2.0*offset));

  vec4 _tl3 = texture2D(tex, texCoord + vec2( -2.0*offset,      offset));
  vec4 _tr3 = texture2D(tex, texCoord + vec2(  2.0*offset,      offset));


  // Laplace of Gaussian filter kernel
  gl_FragColor=abs(1.0/64.0 * (
    1.0 *_tl1 +  4.0 *_tl2 +  6.0 *_t  + 4.0 *_tr2 + 1.0 *_tr1 +
    4.0 *_tl3 +  0.0 *tl   + -8.0 *t   + 0.0 *tr   + 4.0 *_tr3 +
    6.0 *_l   + -8.0 *l    +-28.0 *c   +-8.0 *r    + 6.0 *_r   +
    4.0 *_bl3 *  0.0 *bl   + -8.0 *b   + 0.0 *br   + 4.0 *_br3 +
    1.0 *_bl1 +  4.0 *_bl2 +  6.0 *_b  + 4.0 *_br2 + 1.0 *_br1
  ));
*/

}


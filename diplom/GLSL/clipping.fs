/*//////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 8138 $
*/


/*/ the texture unit to use */
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

/*/ the size of the texture */
uniform float textureSizeW;
uniform float textureSizeH;


/*/ blending parameter for blending in the color coded tensors */
uniform float colormapBlend;


/*////////////////////////////////////////////////////////////////////////////////////////////
// Clipping -- fragment shader -- main
//
// Gets called for every fragment and uses color of first texture and combines it with the
// depth map to clip away noise outside of the geometry image
////////////////////////////////////////////////////////////////////////////////////////////*/
void main()
{
  /*////////////////////////////////////////////////////////////////////////////////////////////
  // GET DATA
  //
  // Retrieve data from the textures
  ////////////////////////////////////////////////////////////////////////////////////////////*/

  vec4 image   = texture2D(tex0, gl_TexCoord[0].st);
  vec4 depth   = texture2D(tex1, gl_TexCoord[1].st);
  vec4 tensor  = texture2D(tex2, gl_TexCoord[2].st);
  vec4 edge    = texture2D(tex3, gl_TexCoord[3].st);

  /*////////////////////////////////////////////////////////////////////////////////////////////
  // DEPTH CLIP
  //
  // Use depth information to cut off background
  ////////////////////////////////////////////////////////////////////////////////////////////*/

  if (depth.r >= 0.99)
    discard;/*/    gl_FragColor=vec4(0.0, 0.0, 0.0, 1.0); */
  else
    gl_FragColor=((1.0 - colormapBlend) * image) + (colormapBlend * tensor) + edge; /*/ +edge? yes the shader clamps it to 0-1 */

  gl_FragDepth=depth.r;
}


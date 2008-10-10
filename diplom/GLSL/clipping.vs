/*//////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 8038 $


/////////////////////////////////////////////////////////////////////////////////////////////
// Clipping -- vertex shader -- main
//
// Gets called for every vertex and just forwards needed information
////////////////////////////////////////////////////////////////////////////////////////////*/
void main()
{
  /*/ pass the color to the fragment shader */
  gl_FrontColor = gl_Color;
  gl_BackColor =  gl_Color;

  /*/ pass tex coordinates */
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_TexCoord[2] = gl_MultiTexCoord2;
  gl_TexCoord[3] = gl_MultiTexCoord3;

  /*/ transform position */
  gl_Position = ftransform();
}


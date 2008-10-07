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



/////////////////////////////////////////////////////////////////////////////////////////////
// Transform -- vertex shader -- main
//
// Gets called for every vertex and uses the color as tensor and transforms it into image
// space. All interpolation is done by GPU.
/////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
  
  vec4 tensor=gl_Color;

  /////////////////////////////////////////////////////////////////////////////////////////////
  // UNSCALE TENSOR
  //
  // Unscale tensor data using given min max information
  /////////////////////////////////////////////////////////////////////////////////////////////

  tensor.x = (tensor.r * (maxX - minX)) + minX;
  tensor.y = (tensor.g * (maxY - minY)) + minY;
  tensor.z = (tensor.b * (maxZ - minZ)) + minZ;
  tensor.w = 1.0;    // w component



  /////////////////////////////////////////////////////////////////////////////////////////////
  // TRANSFORM INTO IMAGE SPACE
  //
  // Use gl_ModelViewProjectionMatrix to transform the tensor to image space
  /////////////////////////////////////////////////////////////////////////////////////////////

  // project
  vec4 base=gl_Vertex;
  vec4 diff=gl_Vertex + tensor;
  diff.w=1.0;
  
  vec4 diffP=gl_ModelViewProjectionMatrix * diff;
  vec4 baseP=gl_ModelViewProjectionMatrix * base;

  // scale by w
  diffP.x /= diffP.w;
  diffP.y /= diffP.w;

  baseP.x /= baseP.w;
  baseP.y /= baseP.w;

  tensor= diffP-baseP;

  // map to [0,1]
  tensor.x = tensor.x * 0.5 + 0.5;
  tensor.y = tensor.y * 0.5 + 0.5;
  tensor.z = 0.0; //tensor.z * 0.5 + 0.5; 

  
  // pass the color to the fragment shader
  gl_FrontColor = tensor;
  gl_BackColor  = tensor;

  // allow usage of clipping planes
  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
  
  // transform position
  gl_Position = ftransform();
}


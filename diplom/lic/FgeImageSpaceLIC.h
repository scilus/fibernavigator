///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 8242 $


#ifndef FgeImageSpaceLIC_hh
#define FgeImageSpaceLIC_hh

#include <GL/glew.h>
#include "FgeOffscreen.h"
#include "FgeGLTexture.h"

#include "../Shader.h"
#include "../datasetInfo.h"
#include "../DatasetHelper.h"

#include <vector>
#include <string>

class DatasetInfo;

/** This class implements an Image Space Based LIC Algorithm as described in

    \ingroup Fge
 */
class  FgeImageSpaceLIC
{
  public:
    FgeImageSpaceLIC(DatasetHelper* dh);
    virtual ~FgeImageSpaceLIC();

    virtual void initiate();

    virtual void render(DatasetInfo* info);

    //! sets the minima and maxima used for scaling the color values.
    void setMinMax(float minX, float minY, float minZ,
                   float maxX, float maxY, float maxZ);

    //! sets the iterations to use for LIC
    void setIterations(unsigned int it);

    //! returns the currently set iteration count
    unsigned int getIterations();

    //! sets the blending factor for noise blending
    void setNoiseBlend(float nb);

    //! returns the set noise blending factor
    float getNoiseBlend();

    //! sets the blending factor for blending in the color coded tensors
    void setColormapBlend(float cb);

    //! returns the set color coded tensor blending factor
    float getColormapBlend();

    //! sets the animation flag
    void setAdvectionAnimation(bool aa);

    //! returns the animation flag
    bool getAdvectionAnimation();

    //! sets the scaling value for scaling tensors for advection
    void setTensorAdvectionScale(float tas);

    //! returns the currently set scaling value
    float getTensorAdvectionScale();


  private:

	DatasetHelper* m_dh;

    float minX, minY, minZ, maxX, maxY, maxZ;

    //! the transform shader program
    Shader* m_transformShader;

    //! the edge detection shader program
    Shader* m_edgeShader;

    //! the advection shader program
    Shader* m_advectionShader;

    //! the clipping shader program
    Shader* m_clippingShader;

    //! reloads and binds the shaders. also dumps errors
    void reloadShaders();

  private:

    //! number of iterations
    unsigned int iterations;

    //! noise blending factor for "refreshing" noise during advection steps
    float noiseBlend;

    //! blending factor for blending in the color coded tensors
    float colormapBlend;

    //! scaler for the tensors (rescale tensors)
    float tensorAdvectionScale;

    //! particle flow animation
    bool advectionAnimation;

    //! old advection texture
    FgeGLTexture* previousAdvectedTexture;

    //! the noise texture
    FgeGLTexture* noiseTexture;

    //! show intermdiate textures? (results and inputs of the individual steps)
    bool debugWindow;

    //! size of the additional surrounding area for rendering ( to avoid artefacts)
    int frameSize;
};

#endif /* FgeImageSpaceLIC_hh */


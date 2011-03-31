///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile: $
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 9218 $


#ifndef FgeOffscreen_hh
#define FgeOffscreen_hh


#include "FgeFramebufferObject.h"
#include "FgeGLTexture.h"

#include <vector>


/** This class implements a helper class for simply rendering into textures.

    \ingroup Fge
 */
class FgeOffscreen
{
  public:

    FgeOffscreen(unsigned int offscreenDimension);
    FgeOffscreen(unsigned int width, unsigned int height, bool autoScale=false);
    virtual ~FgeOffscreen();


    //! activates the FBO
    void activate();

    //! activate the FBO and select multiple rendertargets
    void activate(unsigned int num, unsigned int texList[]);


    //! deactivates the FBO
    void deactivate();

    //! select texture to render to
    void selectTexture(unsigned int no);

    //! select depth texture to render to
    void selectDepthTexture(unsigned int no);

    //! add a new texture
    unsigned int addTexture(bool permanent=false);

    //! add a new depth texture
    unsigned int addDepthTexture(bool permanent=false);


    //! set default background color
    void setClearColor(const float r, const float g, const float b);

    //! get current clear color
    float* getClearColor();



    //! returns the opengl id of the currently selected texture
    GLuint getTexID();

    //! returns the opengl id of the texture with the given number
    GLuint getTexID(unsigned int no);

    //! returns the opengl id of the currently selected depthtexture
    GLuint getDepthTexID();

    //! returns the opengl id of the depthtexture with the given number
    GLuint getDepthTexID(unsigned int no);

    //! returns the FgeGLTexture Object corresponding to the given id
    FgeGLTexture* getTexObject(unsigned int no);

    //! returns the FgeGLTexture Object corresponding to the given id
    FgeGLTexture* getDepthTexObject(unsigned int no);


    //! just render the selected texture to a quad on a viewport with fbo's size
    void renderTexture();

    //! just render the selected depth texture to a quad on a viewport with fbo's size
    void renderDepthTexture();

    //! just render the texture with given number to a quad on a viewport with fbo's size
    void renderTexture(unsigned int no);

    //! just render the depth texture with given number to a quad on a viewport with fbo's size
    void renderDepthTexture(unsigned int no);

    //! renders the given texture as quad on a viewport with fbo's size
    void renderArbitraryTexture(GLuint id);

    //! renders the given textures as quad on a viewport with fbo's size using multitexturing
    void renderArbitraryTextures(std::vector<GLuint> &ids);

    //! renders the given textures as quad on a viewport with given size
    void renderArbitraryTextures(std::vector<GLuint> &ids, unsigned int width, unsigned int height);

    //! renders the given textures as quad on a viewport with given size, just renders given area
    void renderArbitraryTextures(std::vector<GLuint> &ids, unsigned int width, unsigned int height, unsigned int areaWidth, unsigned int areaHeight);

    //! renders the given textures as quad on a viewport with given size, just renders given area, and offsets the area
    void renderArbitraryTextures(std::vector<GLuint> &ids, unsigned int width, unsigned int height, unsigned int areaWidth, unsigned int areaHeight, int areaXOffset, int areaYOffset);

    //! returns real texture width
    unsigned int getTextureWidth();

    //! returns real texture height
    unsigned int getTextureHeight();

    //! returns used texture area width
    unsigned int getTextureAreaWidth();

    //! returns used texture area height
    unsigned int getTextureAreaHeight();


    //! sets a given textures permanent flag
    void setTexturePermanent(unsigned int no, bool permanent=true);

    //! returns given textures permanent flag
    bool getTexturePermanent(unsigned int no);

    //! sets a given depth textures permanent flag
    void setDepthTexturePermanent(unsigned int no, bool permanent=true);

    //! returns given textures permanent flag
    bool getDepthTexturePermanent(unsigned int no);


  private:

    struct TextureDescriptor
    {
      //! is texture permanent?
      bool permanent;

      //! the texture
      FgeGLTexture* texture;

    };

    //! the width of the fbo
    unsigned int sizeW;
    //! the height of the fbo
    unsigned int sizeH;

    //! width of the rectangle used in texture
    unsigned int usedW;

    //! height of the rectangle used in texture
    unsigned int usedH;

    //! the fbo object
    FgeFramebufferObject* fbo;

    //! used for restoring viewport on disabling
    GLint viewportParams[4];

    //! list of textures
    std::vector<TextureDescriptor> textures;

    //! list of depth textures
    std::vector<TextureDescriptor> depthTextures;

    //! currently selected texture
    GLuint texID;

    //! currently selected depth texture
    GLuint depthID;

    //! the color to use for glClearColor
    float* clearColor;

};

#endif /* FgeOffscreen */


///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2008-04-24 08:52:47 +0200 (Do, 24 Apr 2008) $
         //             Author   :   $Author: hlawit $
//////////              Revision :   $Revision: 9222 $

#ifndef FgeGLTexture_hh
#define FgeGLTexture_hh

#include <GL/glew.h>

#ifdef __WXMSW__
    #include "stdio.h"
#endif
/**
 * A class for Fge internal handling of textures that is much more lightweight compared to
 * FgeTexture
 */
class FgeGLTexture
{
  public:
    FgeGLTexture( GLenum target = GL_TEXTURE_2D, int width=512, int height=512, bool depth=false, GLint type=GL_UNSIGNED_BYTE, GLvoid* data=0);

    ~FgeGLTexture();

    void initialize();

    void glTexParameteri( GLenum what, GLenum how);

    void bind();

    GLuint getId() const { return id; }

    /**
     * \par Description
     * Returns the width of this texture.
     * \return width
     */
    unsigned int getWidth() const { return width; };


    /**
     * \par Description
     * Returns the height of this texture.
     * \return height
     */
    unsigned int getHeight() const { return height; };

    //! save the image in the opengl texture to a ppm
    void saveImageToPPM(const char* filename);


  private:
    GLenum target;
    GLuint id;
    GLint type;

    unsigned int width, height;

    //! true if texture should be used for storing depth buffer information
    bool depth;

    GLvoid* data;
};

#endif


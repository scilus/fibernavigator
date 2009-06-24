///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2007-05-23 16:30:58 +0200 (Mi, 23 Mai 2007) $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 7848 $


#ifndef FgeRenderbuffer_hh
#define FgeRenderbuffer_hh

#include <GL/glew.h>

class FgeRenderbuffer
{
  public:
    FgeRenderbuffer();
    FgeRenderbuffer( GLenum internalFormat, int width, int height );
    ~FgeRenderbuffer();

    void bind();
    void unbind();
    void set( GLenum internalFormat, int width, int height );

    GLuint getId() const;

    static GLint getMaxSize();

  private:
    GLuint m_bufId;
    static GLuint _createBufferId();
};

#endif


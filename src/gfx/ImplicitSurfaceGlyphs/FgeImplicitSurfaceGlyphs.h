//Copyright (c) 2008 Aaron Knoll, Younis Hijazi, Andrew Kensler, Mathias Schott, Charles Hansen and Hans Hagen
//
// edited 2009 by Mario Hlawitschka and Alexander Wiebel
//
//Permission is hereby granted, free of charge, to any person
//obtaining a copy of this software and associated documentation
//files (the "Software"), to deal in the Software without
//restriction, including without limitation the rights to use,
//copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the
//Software is furnished to do so, subject to the following
//conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
#ifndef FgeImplicitSurfaceGlyphs_hh
#define FgeImplicitSurfaceGlyphs_hh
//a#include "FgeGeometry.hh"

#include "FgeGL.h"
//a#include <Fge/src/FgeGLTexture.hh>

//a#include "FgeStripsCore.hh"
#include "../../misc/lic/FgeGLTexture.h"
#include "FgeGLFramebufferExtensions.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include <memory>
#include <string.h>



/** This class implements an GPU based method for rendering super quadrics.

    \ingroup Fge
 */
class  FgeImplicitSurfaceGlyphs
{
  public:
    FgeImplicitSurfaceGlyphs();
    virtual ~FgeImplicitSurfaceGlyphs();

    void setPosition( std::vector<GLfloat> pos )
    {
	    this->pos.swap(pos);
    }
    void setFunction(const std::string &f, float iso,
		   const std::string &dx, const std::string &dy, const std::string &dz);


    virtual void initiate();

    virtual void render();


    //! Add a tensor
    virtual void setNewVertex(GLfloat x, GLfloat y, GLfloat z);

   // void setNewVertex(GLfloat x, GLfloat y, GLfloat z);

    std::vector<GLfloat> pos;

  protected:

    // GL extension pointers
    PFNGLMULTITEXCOORD3FARBPROC pglMultiTexCoord3f;
    PFNGLMULTITEXCOORD4FARBPROC pglMultiTexCoord4f;
/*
    PFNGLBINDFRAMEBUFFERPROC pglBindFramebuffer;
    PFNGLBINDRENDERBUFFERPROC pglBindRenderbuffer;
    PFNGLGENRENDERBUFFERSPROC pglGenRenderbuffers;
    PFNGLFRAMEBUFFERRENDERBUFFER pglFramebufferRenderbuffer;
*/
    FgeGLFramebufferExtensions glext;


  private:

    //! opengl display list id
    GLuint dList;

    //! true if the displaylists should be recreated
    bool recreateList;


	//! does the actual rendering pass
	void raycasting_pass();

  public:

    //! sharpness parameter
    void setGamma( float gamma );

    //! rescale tensor glyphs
    void setScaling( float scale );

    //! TODO
    void setData( const unsigned int maxNumberOfGlyphs, GLfloat* param, unsigned int width, unsigned int height );

    //! Create a texture from the data that has been set.
    void dataToTexture( unsigned int width, unsigned int height );

    //! unify scaling to largest value
    void setUnifiedScaling( bool ulv);

    //! set rendermode
    void setRenderMode(unsigned int mode);

    //! how many modes does this primitive offer?
    static unsigned int getNbRenderModes();

    //! get name of rendermode
    static std::string getRenderModeName( unsigned int i );
    void setColorDirections(bool doColor){ color_direction = doColor; }

  protected:

    float boxScale;
    bool unifiedScaling;

    unsigned int renderMode;

    static char const * const RENDERMODES[];
//    static const char * const RENDERMODE_DEFINES[];
//

    //void vertex( const Vector3& v );
    //void vertex( float x, float y, float z );
    void cubeVertex( float x, float y, float z, float tx, float ty, float tz, float glyphPos );
    //void vertex( float x, float y, float z, const Vector3& nearPoint, const Vector3& cameraPlaneDir );
    void enable_renderbuffers();
    void disable_renderbuffers();
    void init();
    void drawBox( double x, double y, double z, double s, int glyph );
        std::string lookupString( int nb ) const;
    void build_shader( int control );
    void mymain(/*int argc, char**argv*/);
    bool color_direction;

  private:
      std::vector<std::vector<double> > vertices;
      FgeGLTexture *tex;
      GLfloat* data;
      unsigned int maxNumberOfGlyphs;
};

#endif /* FgeImplicitSurfaceGlyphs_hh */


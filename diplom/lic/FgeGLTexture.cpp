///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2008-04-24 08:52:47 +0200 (Do, 24 Apr 2008) $
         //             Author   :   $Author: hlawit $
//////////              Revision :   $Revision: 9222 $

#include "FgeGLTexture.h"

#include <fstream>
#include <iostream>

#include <cassert>

using namespace std;

/**
 * \par Description:
 *      Saves the Texture Image to a PPM file.
 *
 * \param filename the filename to save to
 */
void FgeGLTexture::saveImageToPPM(const char* filename)
{
  // FIXME:
  // defined by opengl manual, we should do something like
  // glGetTexLevelParameter and GL_PACK ALIGNMENT check here
  if ( type == GL_UNSIGNED_BYTE )
  {
    // allocate mem
    GLubyte *imageData= new GLubyte[width * height * 4];

    // get image
    bind();
    glGetTexImage( target, 0, GL_RGBA, type, imageData);

    // finally write
    ofstream o( filename );
    o << "P3\n# Texture\n"<< width << " " << height << "\n255\n";

    for(unsigned int i=0; i< width*height; ++i)
    {
      o << (unsigned int)imageData[i*4+0] << " " << (unsigned int)imageData[i*4+1] << " " << (unsigned int)imageData[i*4+2]<< "\n";
    }

    o.close();
    delete[] imageData;
  }
  else if ( type == GL_BYTE )
  {
    // allocate mem
    GLbyte *imageData= new GLbyte[width * height * 4];

    // get image
    bind();
    glGetTexImage( target, 0, GL_RGBA, type, imageData);

    // finally write
    ofstream o( filename );
    o << "P3\n# Texture\n"<< width << " " << height << "\n255\n";

    for(unsigned int i=0; i< width*height; ++i)
    {
      o << (unsigned int)imageData[i*4+0] << " " << (unsigned int)imageData[i*4+1] << " " << (unsigned int)imageData[i*4+2]<< "\n";
    }

    o.close();
    delete[] imageData;
  }
  else if ( type == GL_FLOAT )
  {
    // allocate mem
    GLfloat *imageData= new GLfloat[width * height * 4];

    // get image
    bind();
    glGetTexImage( target, 0, GL_RGBA, type, imageData);

    // finally write
    ofstream o( filename );
    o << "P3\n# Texture\n"<< width << " " << height << "\n10000\n";

    for(unsigned int i=0; i< width*height; ++i)
    {
      o << (int )( 10000.*imageData[i*4+0] ) << " " << (int)( 10000.*imageData[i*4+1] ) << " " << (int)( 10000.*imageData[i*4+2] )<< "\n";
    }

    o.close();
    delete[] imageData;
  }
}

namespace{
  /** extract RGB data out of an RGBA thingy and save it to the (binary)stream */
  template<class T>
  void writeRGB( std::ostream& o, T*data, const size_t size )
  {
    const char* d = ( const char* )data;
    for ( int i=0; i< size; i+=( 4*sizeof( T ) ) )
    {
      o.write( &d[ i ], 3*sizeof( T ) );
    }
  }
}

FgeGLTexture::FgeGLTexture( GLenum target, int width, int height, bool depth, GLint type, GLvoid* data)
  : target( target ), type( type ), width(width), height(height), depth(depth), data( data ){}

FgeGLTexture::~FgeGLTexture()
{
  glDeleteTextures(1, &id);
}

void FgeGLTexture::initialize()
{
  glGenTextures(1, &id);
  glBindTexture(target, id);
  if ( target == GL_TEXTURE_RECTANGLE_ARB )
  {
    // RECTANGLE extension currently only supports GL_NEAREST
    glTexParameteri( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   }
  else
  {
    glTexParameteri( GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  if (depth)
    glTexImage2D(target, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, type, data);
  else
  {
    if ( type == GL_UNSIGNED_BYTE || type == GL_BYTE )
      glTexImage2D(target, 0, GL_RGBA8, width, height, 0, GL_RGBA, type, data );
    else
    {
      if ( target == GL_TEXTURE_2D )
      {
        glTexImage2D( target, 0, GL_RGBA16, width, height, 0, GL_RGBA, type, data );
      }
      else if (  target == GL_TEXTURE_RECTANGLE_ARB )
      {
        glTexImage2D( target, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA, type, data );
      }
    }
  }
}

void FgeGLTexture::glTexParameteri( GLenum what, GLenum how)
{
  ::glTexParameteri( target, what, how);
}

void FgeGLTexture::bind()
{
  //DUMP_GL_ERROR( "FgeGLTexture::bind(): there already is an error" );
  glBindTexture( target, id );
  //DUMP_GL_ERROR( "FgeGLTexture::bind(): glBindTexture" );
}



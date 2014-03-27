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

#include <png.h>
using namespace std;


void FgeGLTexture::saveImageToPNG(const char* filename, bool isEnableAlpha, bool isFlipAlpha)
{
    
    int nbrChannels = 3;
    FILE *fp = fopen(filename,"wb");
    if (!fp)
    {
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if(!png_ptr)
        return;

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if(!png_ptr)
    {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);

    if (isEnableAlpha)
    {
        png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, 
            PNG_INTERLACE_NONE, 
            PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
        nbrChannels = 4;
    }
    else
    {
        png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE, 
            PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
    }

    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
        return;

    if (isFlipAlpha && isEnableAlpha)
    {
        png_set_invert_alpha(png_ptr);
    }

    png_byte **row_pointers = new png_byte*[height];

    // allocate mem
    GLubyte *tmpImageData= new GLubyte[width * height * nbrChannels];
    GLubyte *imageData= new GLubyte[width * height * nbrChannels];

    // get image
    bind();
    if (isEnableAlpha)
    {
        glGetTexImage( target, 0, GL_RGBA, type, tmpImageData);
    }
    else
    {
        glGetTexImage( target, 0, GL_RGB, type, tmpImageData);
    }

    unsigned char* pRow;
    for ( unsigned int i = 1 ; i <= height ; ++i )
    {
        for ( unsigned int j = 0 ; j < width * nbrChannels ; j++ )
        {                    
            imageData[(i-1) * width  + j ] = tmpImageData[(height - i) * width * nbrChannels + j ];
        }
        pRow = &imageData[(i-1) * width];
        png_write_row(png_ptr, pRow);
    }    

    if (setjmp(png_jmpbuf(png_ptr)))
        return;


    png_write_end(png_ptr, info_ptr);
    png_data_freer(png_ptr, info_ptr, PNG_DESTROY_WILL_FREE_DATA, PNG_FREE_PLTE | PNG_FREE_TRNS | PNG_FREE_HIST);
    fclose(fp);
    delete[] imageData;
    delete[] tmpImageData;
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



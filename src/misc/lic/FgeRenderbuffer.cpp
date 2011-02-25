///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2007-05-23 16:30:58 +0200 (Mi, 23 Mai 2007) $
         //             Author   :   $Author: ebaum $
//////////              Revision :   $Revision: 7848 $

#include "FgeRenderbuffer.h"

#include <iostream>
using namespace std;

FgeRenderbuffer::FgeRenderbuffer()
  : m_bufId(_createBufferId())
{}

FgeRenderbuffer::FgeRenderbuffer(GLenum internalFormat, int width, int height)
  : m_bufId(_createBufferId())
{
  set(internalFormat, width, height);
}

FgeRenderbuffer::~FgeRenderbuffer()
{
  glDeleteRenderbuffersEXT(1, &m_bufId);
}

void FgeRenderbuffer::bind()
{
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_bufId);
}

void FgeRenderbuffer::unbind()
{
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}

void FgeRenderbuffer::set(GLenum internalFormat, int width, int height)
{
  int maxSize = FgeRenderbuffer::getMaxSize();
  if (width > maxSize || height > maxSize ) {
    cerr << "FgeRenderbuffer::FgeRenderbuffer() ERROR:\n\t"
         << "Size too big (" << width << ", " << height << ")\n";
    return;
  }

  // Guarded bind
  GLint savedId = 0;
  glGetIntegerv( GL_RENDERBUFFER_BINDING_EXT, &savedId );
  if (savedId != (GLint)m_bufId) {
    bind();
  }

  // Allocate memory for renderBuffer
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, width, height );

  // Guarded unbind
  if (savedId != (GLint)m_bufId) {
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, savedId);
  }
}

GLuint FgeRenderbuffer::getId() const
{
  return m_bufId;
}

GLint FgeRenderbuffer::getMaxSize()
{
  GLint maxAttach = 0;
  glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE_EXT, &maxAttach );
  return maxAttach;
}

GLuint FgeRenderbuffer::_createBufferId()
{
  GLuint id = 0;
  glGenRenderbuffersEXT(1, &id);
  return id;
}



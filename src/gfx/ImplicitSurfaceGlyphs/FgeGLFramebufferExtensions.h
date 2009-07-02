#ifndef FgeGLFramebufferExtensions_hh
#define FgeGLFramebufferExtensions_hh


#include "FgeGL.h"
#include "FgeGLExtensions.h"

struct FgeGLFramebufferExtensions
{
  FgeGLFramebufferExtensions(){}

  // internal pointers
  PFNGLISRENDERBUFFEREXTPROC             pglIsRenderbufferEXT;
  PFNGLDELETERENDERBUFFERSEXTPROC        pglDeleteRenderbuffersEXT;
  PFNGLGENRENDERBUFFERSEXTPROC           pglGenRenderbuffersEXT;
  PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC pglGetRenderbufferParameterivEXT;
  PFNGLISFRAMEBUFFEREXTPROC              pglIsFramebufferEXT;
  PFNGLBINDFRAMEBUFFEREXTPROC            pglBindFramebufferEXT;
    PFNGLBINDRENDERBUFFEREXTPROC           pglBindRenderbufferEXT;
  PFNGLDELETEFRAMEBUFFERSEXTPROC         pglDeleteFramebuffersEXT;
  PFNGLGENFRAMEBUFFERSEXTPROC            pglGenFramebuffersEXT;
  PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC     pglCheckFramebufferStatusEXT;
  PFNGLFRAMEBUFFERTEXTURE1DEXTPROC       pglFramebufferTexture1DEXT;
  PFNGLFRAMEBUFFERTEXTURE2DEXTPROC       pglFramebufferTexture2DEXT;
  PFNGLFRAMEBUFFERTEXTURE3DEXTPROC       pglFramebufferTexture3DEXT;
  PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC    pglFramebufferRenderbufferEXT;
  PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC pglGetFramebufferAttachmentParameterivEXT;
  PFNGLGENERATEMIPMAPEXTPROC             pglGenerateMipmapEXT;
  PFNGLRENDERBUFFERSTORAGEEXTPROC             pglRenderbufferStorageEXT;

  void initialize( void );

};

#endif

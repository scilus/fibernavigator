///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2008-02-07 16:38:12 +0100 (Do, 07 Feb 2008) $
//             Author   :   $Author: wiebel $
//////////              Revision :   $Revision: 8815 $

#ifndef FgeFramebufferObject_hh
#define FgeFramebufferObject_hh

#include <GL/glew.h>

#include <iosfwd>
#include <iostream>

class FgeFramebufferObject {
public:
	//! by default do not call initialize, because we
	//! need an opengl context for that
	FgeFramebufferObject(bool initialize = false);
	virtual ~FgeFramebufferObject();

	void initialize();

	void bind();

	virtual void attachTexture(GLenum texTarget, GLuint textureId,
			GLenum attachment = GL_COLOR_ATTACHMENT0_EXT, int mipLevel = 0,
			int zSlice = 0);

	virtual void attachTextures(int numTextures, GLenum textureTarget[],
			GLuint textureId[], GLenum attachment[] = NULL, int mipLevel[] =
					NULL, int zSlice[] = NULL);

	/// Bind a render buffer to the "attachment" point of this FBO
	virtual void attachRenderBuffer(GLuint buffId, GLenum attachment =
			GL_COLOR_ATTACHMENT0_EXT);

	/// Bind an array of render buffers to corresponding "attachment" points
	/// of this FBO.
	/// - By default, the first 'numBuffers' attachments are used,
	///   starting with GL_COLOR_ATTACHMENT0_EXT
	virtual void attachRenderBuffers(int numBuffers, GLuint buffId[],
			GLenum attachment[] = NULL);

	/// Free any resource bound to the "attachment" point of this FBO
	void unattach(GLenum attachment);

	/// Free any resources bound to any attachment points of this FBO
	void unattachAll();

	/// Is this FBO currently a valid render target?
	///  - Sends output to std::cerr by default but can
	///    be a user-defined C++ stream
	///
	/// NOTE : This function works correctly in debug build
	///        mode but always returns "true" if NODEBUG is
	///        is defined (optimized builds)
#ifndef NODEBUG
	bool isValid(std::ostream& ostr = std::cerr);
#else
	bool isValid( std::ostream& ostr = std::cerr ) {
		return true;
	}
#endif

	/// BEGIN : Accessors
	/// Is attached type GL_RENDERBUFFER_EXT or GL_TEXTURE?
	GLenum getAttachedType(GLenum attachment);

	/// What is the Id of Renderbuffer/texture currently
	/// attached to "attachement?"
	GLuint getAttachedId(GLenum attachment);

	/// Which mipmap level is currently attached to "attachement?"
	GLint getAttachedMipLevel(GLenum attachment);

	/// Which cube face is currently attached to "attachment?"
	GLint getAttachedCubeFace(GLenum attachment);

	/// Which z-slice is currently attached to "attachment?"
	GLint getAttachedZSlice(GLenum attachment);
	/// END : Accessors


	/// BEGIN : Static methods global to all FBOs
	/// Return number of color attachments permitted
	static GLint getMaxColorAttachments();

	/// Disable all FBO rendering and return to traditional,
	/// windowing-system controlled framebuffer
	///  NOTE:
	///     This is NOT an "unbind" for this specific FBO, but rather
	///     disables all FBO rendering. This call is intentionally "static"
	///     and named "Disable" instead of "Unbind" for this reason. The
	///     motivation for this strange semantic is performance. Providing
	///     "Unbind" would likely lead to a large number of unnecessary
	///     FBO enablings/disabling.
	static void disable();
	/// END : Static methods global to all FBOs

protected:
	void _GuardedBind();
	void _GuardedUnbind();
	void _FramebufferTextureND(GLenum attachment, GLenum texTarget,
			GLuint texId, int mipLevel, int zSlice);
	static GLuint _GenerateFboId();

private:
	GLuint m_fboId;
	GLint m_savedFboId;
	//static FgeGLFramebufferExtensions *gl;
};

#endif


///////////////////////////////////////////////////////////////////////////////
//
///// /////  ////       Project  :   FAnToM
//   //  // //  //      Module   :   Fge (Rendering and Viewer Components)
//   //  // /////       File     :   $RCSfile:$
//   //  // //          Language :   C++
//    /////  ////       Date     :   $Date: 2008-02-07 16:38:12 +0100 (Do, 07 Feb 2008) $
//             Author   :   $Author: wiebel $
//////////              Revision :   $Revision: 8815 $


#include "FgeFramebufferObject.h"

using namespace std;

//FgeGLFramebufferExtensions *FgeFramebufferObject::gl = 0L;

FgeFramebufferObject::FgeFramebufferObject(bool initializeMe) :
	m_savedFboId(0) {
	//if (!gl) {
		//gl = new FgeGLFramebufferExtensions;
		initialize();
	//}

	m_fboId = _GenerateFboId();
	if (initializeMe)
		initialize();
}

void FgeFramebufferObject::initialize() {
	// Bind this FBO so that it actually gets created now
	_GuardedBind();
	_GuardedUnbind();
}

FgeFramebufferObject::~FgeFramebufferObject() {

	glDeleteFramebuffersEXT(1, &m_fboId);
}

void FgeFramebufferObject::bind() {

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboId);
}

void FgeFramebufferObject::disable() {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FgeFramebufferObject::attachTexture(GLenum texTarget, GLuint texId,
		GLenum attachment, int mipLevel, int zSlice) {
	_GuardedBind();

#ifndef NODEBUG
	if (getAttachedId(attachment) != texId) {
#endif

		_FramebufferTextureND(attachment, texTarget, texId, mipLevel, zSlice);

#ifndef NODEBUG
	} else {
		cerr << "FgeFramebufferObject::attachTexture PERFORMANCE WARNING:\n"
				<< "\tRedundant bind of texture (id = " << texId << ").\n"
				<< "\tHINT : Compile with -DNODEBUG to remove this warning.\n";
	}
#endif

	_GuardedUnbind();
}

void FgeFramebufferObject::attachTextures(int numTextures, GLenum texTarget[],
		GLuint texId[], GLenum attachment[], int mipLevel[], int zSlice[]) {
	for (int i = 0; i < numTextures; ++i) {
		attachTexture(texTarget[i], texId[i], attachment ? attachment[i]
				: (GL_COLOR_ATTACHMENT0_EXT + i), mipLevel ? mipLevel[i] : 0,
				zSlice ? zSlice[i] : 0);
	}
}

void FgeFramebufferObject::attachRenderBuffer(GLuint buffId, GLenum attachment) {
	_GuardedBind();

#ifndef NODEBUG
	if (getAttachedId(attachment) != buffId) {
#endif

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment,
				GL_RENDERBUFFER_EXT, buffId);

#ifndef NODEBUG
	} else {
		cerr
				<< "FgeFramebufferObject::AttachRenderBuffer PERFORMANCE WARNING:\n"
				<< "\tRedundant bind of Renderbuffer (id = " << buffId << ")\n"
				<< "\tHINT : Compile with -DNODEBUG to remove this warning.\n";
	}
#endif

	_GuardedUnbind();
}

void FgeFramebufferObject::attachRenderBuffers(int numBuffers, GLuint buffId[],
		GLenum attachment[]) {
	for (int i = 0; i < numBuffers; ++i) {
		attachRenderBuffer(buffId[i], attachment ? attachment[i]
				: (GL_COLOR_ATTACHMENT0_EXT + i));
	}
}

void FgeFramebufferObject::unattach(GLenum attachment) {
	_GuardedBind();
	GLenum type = getAttachedType(attachment);

	switch (type) {
	case GL_NONE:
		break;
	case GL_RENDERBUFFER_EXT:
		attachRenderBuffer(0, attachment);
		break;
	case GL_TEXTURE:
		attachTexture(GL_TEXTURE_2D, 0, attachment);
		break;
	default:
		cerr
				<< "FgeFramebufferObject::unbind_attachment ERROR: Unknown attached resource type\n";
	}
	_GuardedUnbind();
}

void FgeFramebufferObject::unattachAll() {
	int numAttachments = getMaxColorAttachments();
	for (int i = 0; i < numAttachments; ++i) {
		unattach(GL_COLOR_ATTACHMENT0_EXT + i);
	}
}

GLint FgeFramebufferObject::getMaxColorAttachments() {
	GLint maxAttach = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxAttach);
	return maxAttach;
}

GLuint FgeFramebufferObject::_GenerateFboId() {
	GLuint id = 0;
	//if (gl)
		glGenFramebuffersEXT(1, &id);
	return id;
}

void FgeFramebufferObject::_GuardedBind() {
	// Only binds if m_fboId is different than the currently bound FBO
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_savedFboId);
	if (m_fboId != (GLuint) m_savedFboId) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboId);
	}
}

void FgeFramebufferObject::_GuardedUnbind() {
	// Returns FBO binding to the previously enabled FBO
	if (m_fboId != (GLuint) m_savedFboId) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint) m_savedFboId);
	}
}

void FgeFramebufferObject::_FramebufferTextureND(GLenum attachment,
		GLenum texTarget, GLuint texId, int mipLevel, int zSlice) {
	if (texTarget == GL_TEXTURE_1D) {
		glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment,
				GL_TEXTURE_1D, texId, mipLevel);
	} else if (texTarget == GL_TEXTURE_3D) {
		glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment,
				GL_TEXTURE_3D, texId, mipLevel, zSlice);
	} else {
		// Default is GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, or cube faces
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
				texTarget, texId, mipLevel);
	}
}

#ifndef NODEBUG
bool FgeFramebufferObject::isValid(ostream& ostr) {
	_GuardedBind();

	bool isOK = false;

	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
		isOK = true;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "GL_FRAMEBUFFER_UNSUPPORTED_EXT\n";
		isOK = false;
		break;
	default:
		ostr << "glift::CheckFramebufferStatus() ERROR:\n\t"
				<< "Unknown ERROR\n";
		isOK = false;
	}

	_GuardedUnbind();
	return isOK;
}
#endif // NODEBUG
/// Accessors
GLenum FgeFramebufferObject::getAttachedType(GLenum attachment) {
	// Returns GL_RENDERBUFFER_EXT or GL_TEXTURE
	_GuardedBind();
	GLint type = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
			attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT, &type);
	_GuardedUnbind();
	return GLenum(type);
}

GLuint FgeFramebufferObject::getAttachedId(GLenum attachment) {
	_GuardedBind();
	GLint id = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
			attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT, &id);
	_GuardedUnbind();
	return GLuint(id);
}

GLint FgeFramebufferObject::getAttachedMipLevel(GLenum attachment) {
	_GuardedBind();
	GLint level = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
			attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT, &level);
	_GuardedUnbind();
	return level;
}

GLint FgeFramebufferObject::getAttachedCubeFace(GLenum attachment) {
	_GuardedBind();
	GLint level = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
			attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT,
			&level);
	_GuardedUnbind();
	return level;
}

GLint FgeFramebufferObject::getAttachedZSlice(GLenum attachment) {
	_GuardedBind();
	GLint slice = 0;
	glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
			attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT,
			&slice);
	_GuardedUnbind();
	return slice;
}


/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOVOLUMERENDERING_H
#define COIN_SOVOLUMERENDERING_H

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H*/

#include <GL/gl.h>

//Extensions for compressed textures...
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);

extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	glCompressedTexSubImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	glCompressedTexSubImage1DARB;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	glGetCompressedTexImageARB;

//Extensions for paletted textures...
#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT (0x80E2)
#define GL_COLOR_INDEX2_EXT (0x80E3)
#define GL_COLOR_INDEX4_EXT (0x80E4)
#define GL_COLOR_INDEX8_EXT (0x80E5)
#define GL_COLOR_INDEX12_EXT (0x80E6)
#define GL_COLOR_INDEX16_EXT (0x80E7)
#define GL_TEXTURE_INDEX_SIZE_EXT (0x80ED)

#define GL_COLOR_TABLE_FORMAT_EXT (0x80D8)
#define GL_COLOR_TABLE_WIDTH_EXT (0x80D9)
#define GL_COLOR_TABLE_RED_SIZE_EXT (0x80DA)
#define GL_COLOR_TABLE_GREEN_SIZE_EXT (0x80DB)
#define GL_COLOR_TABLE_BLUE_SIZE_EXT (0x80DC)
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT 80x80DD)
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT (0x80DE)
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT (0x80DF)
#define GL_TEXTURE_INDEX_SIZE_EXT (0x80ED)
#endif 

typedef void (APIENTRY * PFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (APIENTRY * PFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);

extern PFNGLCOLORTABLEEXTPROC glColorTableEXT;
extern PFNGLCOLORSUBTABLEEXTPROC glColorSubTableEXT;
extern PFNGLGETCOLORTABLEEXTPROC glGetColorTableEXT;
extern PFNGLGETCOLORTABLEPARAMETERIVEXTPROC glGetColorTableParameterivEXT;
extern PFNGLGETCOLORTABLEPARAMETERFVEXTPROC glGetColorTableParameterfvEXT;



class SoVolumeRendering : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SoVolumeRendering);

public:
  static void initClass(void);

  enum HW_Feature {
    HW_VOLUMEPRO, 
    HW_3DTEXMAP, 
    HW_TEXCOLORMAP, 
    HW_TEXCOMPRESSION
  };

  enum HW_SupportStatus {
    NO, 
    YES, 
    UNKNOWN
  };

  enum Axis {
    X, 
    Y, 
    Z
  };

  enum DataType {
    UNSIGNED_BYTE,
    UNSIGNED_SHORT,
    RGBA
  };

  SoVolumeRendering();
  ~SoVolumeRendering();
  static void init(); 
  HW_SupportStatus isSupported(HW_Feature feature);

private:
  friend class SoVolumeRenderingP;
  class SoVolumeRenderingP * pimpl;
};//SoVolumeRendering

#endif // !COIN_SOVOLUMERENDERING_H

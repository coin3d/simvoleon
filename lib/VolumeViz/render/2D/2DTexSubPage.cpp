#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/errors/SoDebugError.h>

// (This is cut'n'pasted from torbjorv's end-of-project doc in
// SoVolumeData.cpp:)
//
// PALETTED TEXTURES
//
//   Paletted textures rocks. Depending on the size of the pages, it
//   could save significant amounts of memory. The current implementation
//   uses individual palettes for each page. This can be both a good idea
//   and a terrible one.
//
//   Good: If the video card supports palettes with different sizes. If,
//   for example, a page contains only one color, a 1-bit palette could
//   be used and each pixel will occupy 1 bit of hardware memory.
//
//   Bad: If the video card does NOT support palettes with different
//   sizes. This means that each palette i.e. has to have 256 entries,
//   and with RGBA colors the palette will occupy 256x4=1024 bytes. With
//   page sizes smaller than 64x64, this would make the palette occupy
//   just as many bytes as the actual pixel data. If the video card
//   actually DOES support variable-size palettes, it could still be a
//   bad idea. If all the pages require a 256-entry palette (or more) due
//   to heavy color variations, the palettes would require a lot of
//   hardware memory.
//
//   These problems may be solved by the use of several techniques. First
//   of all, there is an extension called GL_SHARED_PALETTE_EXT, that
//   allows several textures to share the same palette. A global palette
//   for the entire volume could be generated, resulting in some heavy
//   pre-calculation and possibly loss of color accuracy, but saving a
//   lot of memory. The best solution would probably be a combination of
//   local and global palettes. Local, if the page consist entirely of
//   one color. Global and shared whenever heavy color variations occur.
//
// glColorTableEXT
//
//   Study Cvr2DTexSubPage::setData. The code supports palettes of
//   variable sizes, exploiting the obvious advantages explained in the
//   previous section.  In between the uploading of palette and texture,
//   there is a check of what palette size actually achieved. It seems
//   like there's no guarantee that a video card supports the different
//   palette sizes/formats. If the following glTexImage2D tries to set a
//   internal format that doesn't fit the palette size, the entire
//   uploading could fail. At least it does on this card (3DLabs Oxygen
//   GVX1). The check for palette size fixes this problem.



Cvr2DTexSubPage::Cvr2DTexSubPage(const uint8_t * textureimg,
                                 const SbVec2s & size,
                                 const float * palette, int palettesize)
{
  this->texdims = size;

  this->numBytesHW = 0;

  this->transferTex2GL(textureimg, palettesize, palette);
}


Cvr2DTexSubPage::~Cvr2DTexSubPage()
{
  glDeleteTextures(1, this->texturename);
  this->numBytesHW = 0;
}


// FIXME: Some magic has to be done to make this one work with OpenGL 1.0.
// torbjorv 08052002
void
Cvr2DTexSubPage::activate(void) const
{
  glBindTexture(GL_TEXTURE_2D, this->texturename[0]);

#if CVR_DEBUG
  // FIXME: glAreTexturesResident() is OpenGL 1.1 only. 20021119 mortene.
  GLboolean residences[1];
  GLboolean resident = glAreTexturesResident(1, this->texturename, residences);
  if (!resident) {
    SoDebugError::postWarning("Cvr2DTexSubPage::activate",
                              "texture %d not resident", this->texturename);
  }
#endif // CVR_DEBUG

}

// If no palette specified, this function assumes RGBA data. If a
// palette is specified, the input data should be indices into the
// palette.  The function uses the palette's size to decide whether
// the indices are byte or short.
void
Cvr2DTexSubPage::transferTex2GL(const uint8_t * textureimg,
                                int palettesize, const float * palette)
{
  const cc_glglue * glue = cc_glglue_instance(0); // FIXME: need cache context here

  // FIXME: these functions is only supported in opengl 1.1...
  // torbjorv 08052002
  glGenTextures(1, this->texturename);
  glBindTexture(GL_TEXTURE_2D, this->texturename[0]);

  // Uploading standard RGBA-texture
  if (palette == NULL) {
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 4,
                 this->texdims[0],
                 this->texdims[1],
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 textureimg);

    this->numBytesHW += this->texdims[0] * this->texdims[1] * 4;
  }
  // Uploading paletted texture
  else {
    // FIXME: this limitation is of course not good, and should be
    // lifted. It would BTW probably be better to check for this
    // extension somewhere else before trying to make paletted
    // textures. 20021112 mortene.
    assert(cc_glglue_has_paletted_textures(glue) && "can't handle palette-textures");

    // Check size of indices
    int format = GL_UNSIGNED_BYTE;
    if (palettesize > 256)
      format = GL_UNSIGNED_SHORT;

    // Setting palette
    cc_glglue_glColorTableEXT(glue, GL_TEXTURE_2D,
                              GL_RGBA,
                              palettesize,
                              GL_RGBA,
                              GL_FLOAT,
                              palette);

    // Checking what palettesize we actually got
    int actualPaletteSize;
    cc_glglue_glGetColorTableParameterivEXT(glue, GL_TEXTURE_2D,
                                            GL_COLOR_TABLE_WIDTH_EXT,
                                            &actualPaletteSize);

    this->numBytesHW += actualPaletteSize * 4 * 4;


    int internalFormat;
    switch (actualPaletteSize) {
    case     2: internalFormat = GL_COLOR_INDEX1_EXT;
      this->numBytesHW += this->texdims[0] * this->texdims[1] / 8;
      break;
    case     4: internalFormat = GL_COLOR_INDEX2_EXT;
      this->numBytesHW += this->texdims[0] * this->texdims[1] / 4;
      break;
    case    16: internalFormat = GL_COLOR_INDEX4_EXT;
      this->numBytesHW += this->texdims[0] * this->texdims[1] / 2;
      break;
    case   256: internalFormat = GL_COLOR_INDEX8_EXT;
      this->numBytesHW += this->texdims[0] * this->texdims[1];
      break;
    case 65536: internalFormat = GL_COLOR_INDEX16_EXT;
      this->numBytesHW += this->texdims[0] * this->texdims[1] * 2;
      break;
    default:
      // FIXME: this can indeed hit, try for instance SYN_64.vol. If
      // some palette sizes are indeed unsupported by OpenGL, we
      // should probably resize our palette to the nearest
      // upward. 20021106 mortene.
      assert(FALSE && "unknown palette size");
      break;
    }


    // Upload texture
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internalFormat,
                 this->texdims[0],
                 this->texdims[1],
                 0,
                 GL_COLOR_INDEX,
                 format,
                 textureimg);

  }


  // FIXME: Okay. I tried. This GLWrapper-thingy must be spawned right
  // out of hell. I'm really not able to compile it. But these lines
  // need to be fixed for OpenGL 1.0 support. torbjorv 08032002

  // GLenum clamping;
  // const GLWrapper_t * glw = GLWRAPPER_FROM_STATE(state);
  // if (glw->hasTextureEdgeClamp)
  //   clamping = GL_CLAMP_TO_EDGE;
  // else
  //   (GLenum) GL_CLAMP;


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}

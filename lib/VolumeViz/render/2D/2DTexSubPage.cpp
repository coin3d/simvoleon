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


unsigned int Cvr2DTexSubPage::nroftexels = 0;
unsigned int Cvr2DTexSubPage::texmembytes = 0;


Cvr2DTexSubPage::Cvr2DTexSubPage(const uint8_t * textureimg,
                                 const SbVec2s & size,
                                 const float * palette, int palettesize)
{
  this->texmultfactor = 0.0f;

  assert(size[0] >= 0);
  assert(size[1] >= 0);
  this->texdims = size;

  this->transferTex2GL(textureimg, palettesize, palette);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::Cvr2DTexSubPage",
                         "nroftexels => %d, texmembytes => %d",
                         Cvr2DTexSubPage::nroftexels,
                         Cvr2DTexSubPage::texmembytes);
#endif // debug
}


Cvr2DTexSubPage::~Cvr2DTexSubPage()
{
  glDeleteTextures(1, this->texturename);

  const unsigned int nrtexels = this->texdims[0] * this->texdims[1];
  assert(nrtexels <= Cvr2DTexSubPage::nroftexels);
  Cvr2DTexSubPage::nroftexels -= nrtexels;

  unsigned int freetexmem = (unsigned int)
    (float(nrtexels) * this->texmultfactor);
  assert(freetexmem <= Cvr2DTexSubPage::texmembytes);
  Cvr2DTexSubPage::texmembytes -= freetexmem;

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::~Cvr2DTexSubPage",
                         "nroftexels => %d, texmembytes => %d",
                         Cvr2DTexSubPage::nroftexels,
                         Cvr2DTexSubPage::texmembytes);
#endif // debug
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
  // FIXME: need cache context here! 20021120 mortene.
  const cc_glglue * glw = cc_glglue_instance(0);

  // FIXME: these functions is only supported in opengl 1.1...
  // torbjorv 08052002
  glGenTextures(1, this->texturename);
  glBindTexture(GL_TEXTURE_2D, this->texturename[0]);

  const int nrtexels = this->texdims[0] * this->texdims[1];

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

    this->texmultfactor = 4.0f; // 8 bits each R, G, B & A
  }
  // Uploading paletted texture
  else {
    // FIXME: this limitation is of course not good, and should be
    // lifted. It would BTW probably be better to check for this
    // extension somewhere else before trying to make paletted
    // textures. 20021112 mortene.
    assert(cc_glglue_has_paletted_textures(glw) && "can't handle palette-textures");

    // Check size of indices
    int format = GL_UNSIGNED_BYTE;
    if (palettesize > 256)
      format = GL_UNSIGNED_SHORT;

    // Setting palette
    cc_glglue_glColorTableEXT(glw, GL_TEXTURE_2D,
                              GL_RGBA,
                              palettesize,
                              GL_RGBA,
                              GL_FLOAT,
                              palette);

    // Checking what palettesize we actually got
    int actualPaletteSize;
    cc_glglue_glGetColorTableParameterivEXT(glw, GL_TEXTURE_2D,
                                            GL_COLOR_TABLE_WIDTH_EXT,
                                            &actualPaletteSize);

    int colorformat;

    switch (actualPaletteSize) {
    case 2:
      colorformat = GL_COLOR_INDEX1_EXT;
      this->texmultfactor = 1.0f / 8.0f; // 1 bit pr texel
      break;

    case 4:
      colorformat = GL_COLOR_INDEX2_EXT;
      this->texmultfactor = 1.0f / 4.0f; // 2 bits pr texel
      break;

    case 16:
      colorformat = GL_COLOR_INDEX4_EXT;
      this->texmultfactor = 1.0 / 2.0f; // 4 bits pr texel
      break;

    case 256:
      colorformat = GL_COLOR_INDEX8_EXT;
      this->texmultfactor = 1.0f; // 8 bits pr texel
      break;

    case 65536:
      colorformat = GL_COLOR_INDEX16_EXT;
      this->texmultfactor = 2.0f;  // 16 bits pr texel
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
                 colorformat,
                 this->texdims[0],
                 this->texdims[1],
                 0,
                 GL_COLOR_INDEX,
                 format,
                 textureimg);

  }

  Cvr2DTexSubPage::nroftexels += nrtexels;
  Cvr2DTexSubPage::texmembytes += int(float(nrtexels) * this->texmultfactor);

  GLint wrapenum = GL_CLAMP;
  if (cc_glglue_has_texture_edge_clamp(glw)) { wrapenum = GL_CLAMP_TO_EDGE; }

  // FIXME: investigate if this is really what we want. 20021120 mortene.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapenum);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapenum);

  // FIXME: investigate if this is really what we want. 20021120 mortene.
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

unsigned int
Cvr2DTexSubPage::totalNrOfTexels(void)
{
  return Cvr2DTexSubPage::nroftexels;
}

unsigned int
Cvr2DTexSubPage::totalTextureMemoryUsed(void)
{
  return Cvr2DTexSubPage::texmembytes;
}

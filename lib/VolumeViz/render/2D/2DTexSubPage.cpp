#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <VolumeViz/render/2D/CvrTextureObject.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>

#include "texmemfullimg.h"

// (This is cut'n'pasted from torbjorv's end-of-project doc in
// SoVolumeData.cpp:)
//
// PALETTED TEXTURES
//
//   Paletted textures rocks. Depending on the size of the pages, it
//   could save significant amounts of memory. The current
//   implementation uses individual palettes for each page. This can
//   be both a good idea and a terrible one.
//
//   Good: If the video card supports palettes with different
//   sizes. If, for example, a page contains only one color, a
//   2-element palette could be used and each pixel will occupy 1 bit
//   of hardware memory.
//
//   Bad: If the video card does NOT support palettes with different
//   sizes. This means that each palette i.e. has to have 256 entries,
//   and with RGBA colors the palette will occupy 256x4=1024
//   bytes. With page sizes smaller than 64x64, this would make the
//   palette occupy just as many bytes as the actual pixel data. If
//   the video card actually DOES support variable-size palettes, it
//   could still be a bad idea. If all the pages require a 256-entry
//   palette (or more) due to heavy color variations, the palettes
//   would require a lot of hardware memory.
//
//   These problems may be solved by the use of several
//   techniques. First of all, there is an extension called
//   GL_SHARED_PALETTE_EXT, that allows several textures to share the
//   same palette. A global palette for the entire volume could be
//   generated, resulting in some heavy pre-calculation and possibly
//   loss of color accuracy, but saving a lot of memory. The best
//   solution would probably be a combination of local and global
//   palettes. Local, if the page consist entirely of one
//   color. Global and shared whenever heavy color variations occur.
//
// glColorTableEXT
//
//   Study Cvr2DTexSubPage::setData. The code supports palettes of
//   variable sizes, exploiting the obvious advantages explained in
//   the previous section.  In between the uploading of palette and
//   texture, there is a check of what palette size actually
//   resulted. It seems like there's no guarantee that a video card
//   supports the different palette sizes/formats. If the following
//   glTexImage2D tries to set a internal format that doesn't fit the
//   palette size, the entire uploading could fail. At least it does
//   on this card (3DLabs Oxygen GVX1). The check for palette size
//   fixes this problem.


unsigned int Cvr2DTexSubPage::nroftexels = 0;
unsigned int Cvr2DTexSubPage::texmembytes = 0;
GLuint Cvr2DTexSubPage::emptyimgname[1] = { 0 };
SbBool Cvr2DTexSubPage::detectedtextureswapping = FALSE;

Cvr2DTexSubPage::Cvr2DTexSubPage(SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec2s & pagesize,
                                 const SbVec2s & texsize)
{
  this->bitspertexel = 0;

  assert(pagesize[0] >= 0);
  assert(pagesize[1] >= 0);
  assert(coin_is_power_of_two(pagesize[0]));
  assert(coin_is_power_of_two(pagesize[1]));

  assert(texsize[0] <= pagesize[0]);
  assert(texsize[1] <= pagesize[1]);

  // Actual size of texture bitmap.
  this->texdims = pagesize;

  // Calculates part of GL quad and texture to show.
  this->texmaxcoords = SbVec2f(1.0f, 1.0f);
  if (pagesize != texsize) {
    this->texmaxcoords[0] = float(texsize[0]) / float(pagesize[0]);
    this->texmaxcoords[1] = float(texsize[1]) / float(pagesize[1]);
  }

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::Cvr2DTexSubPage",
                         "this->texmaxcoords==[%f, %f]",
                         this->texmaxcoords[0], this->texmaxcoords[1]);
#endif // debug

  this->transferTex2GL(action, texobj);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::Cvr2DTexSubPage",
                         "nroftexels => %d, texmembytes => %d",
                         Cvr2DTexSubPage::nroftexels,
                         Cvr2DTexSubPage::texmembytes);
#endif // debug
}


Cvr2DTexSubPage::~Cvr2DTexSubPage()
{
  if (this->texturename[0] != 0) {
    glDeleteTextures(1, this->texturename);

    const unsigned int nrtexels = this->texdims[0] * this->texdims[1];
    assert(nrtexels <= Cvr2DTexSubPage::nroftexels);
    Cvr2DTexSubPage::nroftexels -= nrtexels;

    unsigned int freetexmem = (unsigned int)
      (float(nrtexels) * float(this->bitspertexel) / 8.0f);
    assert(freetexmem <= Cvr2DTexSubPage::texmembytes);
    Cvr2DTexSubPage::texmembytes -= freetexmem;

#if CVR_DEBUG && 0 // debug
    SoDebugError::postInfo("Cvr2DTexSubPage::~Cvr2DTexSubPage",
                           "nroftexels => %d, texmembytes => %d",
                           Cvr2DTexSubPage::nroftexels,
                           Cvr2DTexSubPage::texmembytes);
#endif // debug
  }
}


// FIXME: Some magic has to be done to make this one work with OpenGL 1.0.
// torbjorv 08052002
void
Cvr2DTexSubPage::activateTexture(Interpolation interpolation) const
{
  if (this->texturename[0] == 0) {
    glBindTexture(GL_TEXTURE_2D, Cvr2DTexSubPage::emptyimgname[0]);
    return;
  }

  glBindTexture(GL_TEXTURE_2D, this->texturename[0]);

  GLenum interp = 0;
  switch (interpolation) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE); break;
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp);
  assert(glGetError() == GL_NO_ERROR);

#if CVR_DEBUG && 0 // debug
  // FIXME: glAreTexturesResident() is OpenGL 1.1 only. 20021119 mortene.
  GLboolean residences[1];
  GLboolean resident = glAreTexturesResident(1, this->texturename, residences);
  if (!resident) {
    SoDebugError::postWarning("Cvr2DTexSubPage::activateTexture",
                              "texture %d not resident", this->texturename);
    Cvr2DTexSubPage::detectedtextureswapping = TRUE;
  }

  // For reference, here's some information from Thomas Roell of Xi
  // Graphics on glAreTexturesResident() from c.g.a.opengl:
  //
  // [...]
  //
  //   With regards to glAreTexturesResident(), this is kind of
  //   tricky. This function returns which textures are currently
  //   resident is HW accessable memory (AGP, FB, TB). It does not
  //   return whether a set of textures could be made resident at a
  //   future point of time. A lot of OpenGL implementations (APPLE &
  //   XiGraphics for example) do cache a texture upon first use with
  //   3D primitive. Hence unless you had used a texture before it
  //   will not be resident. N.b that usually operations like
  //   glBindTexture, glTex*Image and so on will not make a texture
  //   resident for such caching implementations.
  //
  // [...]
  //
  // Additional information from Ian D Romanick (IBM engineer doing
  // Linux OpenGL work):
  //
  // [...]
  //
  //   AreTexturesResident is basically worthless, IMO.  All OpenGL
  //   rendering happens in a VERY high latency pipeline.  When an
  //   application calls AreTexturesResident, the textures may all be
  //   resident at that time.  However, there may already be
  //   primitives in the pipeline that will cause those textures to be
  //   removed from texturable memory before more primitives can be
  //   put in the pipe.
  //
  // [...]
  //
  // 20021201 mortene.

#endif // debug
}

// If no palette specified, this function assumes RGBA data. If a
// palette is specified, the input data should be indices into the
// palette.  The function uses the palette's size to decide whether
// the indices are byte or short.
void
Cvr2DTexSubPage::transferTex2GL(SoGLRenderAction * action,
                                const CvrTextureObject * texobj)
{
  // FIXME: paletted textures are currently disabled. (The following
  // data should eventually be available from CvrTextureObject.)
  // 20021206 mortene.
  const float * palette = NULL;
  const int palettesize = 0;

  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());

  if (Cvr2DTexSubPage::emptyimgname[0] == 0) {
    glGenTextures(1, Cvr2DTexSubPage::emptyimgname);
    glBindTexture(GL_TEXTURE_2D, Cvr2DTexSubPage::emptyimgname[0]);
    // FIXME: never freed. 20021121 mortene.
    
    // Check format of GIMP-exported "texmem full" image.
    assert(coin_is_power_of_two(tex_image.width));
    assert(coin_is_power_of_two(tex_image.height));
    assert(tex_image.bytes_per_pixel == 4);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 tex_image.bytes_per_pixel,
                 tex_image.width, tex_image.height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 tex_image.pixel_data);

    const int texels = tex_image.width * tex_image.height;
    Cvr2DTexSubPage::nroftexels += texels;
    Cvr2DTexSubPage::texmembytes += texels * tex_image.bytes_per_pixel;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  int colorformat;
  int bytes_pr_unit = GL_UNSIGNED_BYTE;

  // Uploading standard RGBA-texture
  if (palette == NULL) {
    colorformat = 4;
    this->bitspertexel = 32; // 8 bits each R, G, B & A
  }
  // Uploading paletted texture
  else {
    // FIXME: this limitation is of course not good, and should be
    // lifted. It would BTW probably be better to check for this
    // extension somewhere else before trying to make paletted
    // textures. 20021112 mortene.
    assert(cc_glglue_has_paletted_textures(glw) && "can't handle palette-textures");

    // Check size of indices
    if (palettesize > 256) bytes_pr_unit = GL_UNSIGNED_SHORT;

    // Setting palette
    // FIXME: does this need to be called _after_ glBindTextures()?
    // 20021121 mortene.
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

    switch (actualPaletteSize) {
    case 2:
      colorformat = GL_COLOR_INDEX1_EXT;
      this->bitspertexel = 1;
      break;

    case 4:
      colorformat = GL_COLOR_INDEX2_EXT;
      this->bitspertexel = 2;
      break;

    case 16:
      colorformat = GL_COLOR_INDEX4_EXT;
      this->bitspertexel = 4;
      break;

    case 256:
      colorformat = GL_COLOR_INDEX8_EXT;
      this->bitspertexel = 8;
      break;

    case 65536:
      colorformat = GL_COLOR_INDEX16_EXT;
      this->bitspertexel = 16;
      break;

    default:
      // FIXME: this can indeed hit, I've seen actualPaletteSize
      // become 8. If some palette sizes are indeed unsupported by
      // OpenGL, we should probably resize our palette to the nearest
      // upward. 20021106 mortene.
      assert(FALSE && "unknown palette size");
      break;
    }
  }

  const int nrtexels = this->texdims[0] * this->texdims[1];
  const int texmem = int(float(nrtexels) * float(this->bitspertexel) / 8.0f);

  // FIXME: limits should be stored in a global texture manager class
  // or some such. 20021121 mortene.
  if (//Cvr2DTexSubPage::detectedtextureswapping ||
      ((nrtexels + Cvr2DTexSubPage::nroftexels) > (16*1024*1024)) ||
      ((texmem + Cvr2DTexSubPage::texmembytes) > (64*1024*1024))) {
#if CVR_DEBUG && 1 // debug
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postInfo("Cvr2DTexSubPage::transferTex2GL",
                             "filled up textures, nrtexels==%d, texmembytes==%d",
                             Cvr2DTexSubPage::nroftexels,
                             Cvr2DTexSubPage::texmembytes);
      first = FALSE;
    }
#endif // debug
    this->texturename[0] = 0;
  }
  else {
    Cvr2DTexSubPage::nroftexels += nrtexels;
    Cvr2DTexSubPage::texmembytes += texmem;

    // FIXME: these functions are only supported in opengl 1.1+...
    // torbjorv 08052002
    glGenTextures(1, this->texturename);
    assert(glGetError() == GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_2D, this->texturename[0]);
    assert(glGetError() == GL_NO_ERROR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 colorformat,
                 this->texdims[0],
                 this->texdims[1],
                 0,
                 palette == NULL ? GL_RGBA : GL_COLOR_INDEX,
                 bytes_pr_unit,
                 // FIXME: must be changed when we support paletted
                 // textures. 20021206 mortene.
                 texobj->getRGBABuffer());
    assert(glGetError() == GL_NO_ERROR);

    GLint wrapenum = GL_CLAMP;
    if (cc_glglue_has_texture_edge_clamp(glw)) { wrapenum = GL_CLAMP_TO_EDGE; }

    // FIXME: investigate if this is really what we want. 20021120 mortene.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapenum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapenum);
    assert(glGetError() == GL_NO_ERROR);
  }
}

void
Cvr2DTexSubPage::render(const SbVec3f & upleft,
                        SbVec3f widthvec, SbVec3f heightvec,
                        Interpolation interpolation) const
{
  this->activateTexture(interpolation);

  glBegin(GL_QUADS);
  glColor4f(1, 1, 1, 1);

  // Scale span of GL quad to match the visible part of the
  // texture. (Border subpages shouldn't show all of the texture, if
  // the dimensions of the dataset are not a power of two, or if the
  // dimensions are less than the subpage size).

  widthvec *= this->texmaxcoords[0];
  heightvec *= this->texmaxcoords[1];

  // Find all corner points of the quad.

  SbVec3f lowleft = upleft + heightvec;
  SbVec3f lowright = lowleft + widthvec;
  SbVec3f upright = upleft + widthvec;

  // Texturecoords are set up so the texture is flipped in the
  // Y-direction, as the volume data and texture map data are oriented
  // in the opposite direction (top-to-bottom) from what the Y axis in
  // the OpenGL coordinate system uses (bottom-to-top).

  glTexCoord2f(0.0f, this->texmaxcoords[1]);
  glVertex3f(lowleft[0], lowleft[1], lowleft[2]);

  glTexCoord2f(this->texmaxcoords[0], this->texmaxcoords[1]);
  glVertex3f(lowright[0], lowright[1], lowright[2]);

  glTexCoord2f(this->texmaxcoords[0], 0.0f);
  glVertex3f(upright[0], upright[1], upright[2]);

  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(upleft[0], upleft[1], upleft[2]);

  glEnd();


  // This is a debugging backdoor: if the environment variable
  // CVR_SUBPAGE_FRAMES is set to a positive integer, a lineset will
  // be drawn around the border of each page.

  static int showsubpageframes = -1;
  if (showsubpageframes == -1) {
    const char * envstr = coin_getenv("CVR_SUBPAGE_FRAMES");
    if (envstr) { showsubpageframes = atoi(envstr) > 0 ? 1 : 0; }
    else showsubpageframes = 0;
  }

  if (showsubpageframes) {
    glDisable(GL_TEXTURE_2D);
    glLineStipple(1, 0xffff);
    glLineWidth(2);

    glBegin(GL_LINE_LOOP);
    glVertex3f(lowleft[0], lowleft[1], lowleft[2]);
    glVertex3f(lowright[0], lowright[1], lowright[2]);
    glVertex3f(upright[0], upright[1], upright[2]);
    glVertex3f(upleft[0], upleft[1], upleft[2]);
    glEnd();

    glEnable(GL_TEXTURE_2D);
  }
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

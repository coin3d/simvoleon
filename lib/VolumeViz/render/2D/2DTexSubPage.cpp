#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <VolumeViz/render/2D/CvrRGBATexture.h>
#include <VolumeViz/render/2D/CvrPaletteTexture.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>

#include "texmemfullimg.h"

// *************************************************************************

// debugging: keep this around until the peculiar NVidia bug with
// 1- or 2-pixel width textures has been analyzed. 20031031 mortene.

#define GLSYNCHRON 0

#if GLSYNCHRON
#define GLCMD(x) do { printf("%s\n", #x); x; glFinish(); } while(0)
#else
#define GLCMD(x) do { x; } while(0)
#endif

// *************************************************************************

unsigned int Cvr2DTexSubPage::nroftexels = 0;
unsigned int Cvr2DTexSubPage::texmembytes = 0;
GLuint Cvr2DTexSubPage::emptyimgname[1] = { 0 };
SbBool Cvr2DTexSubPage::detectedtextureswapping = FALSE;

// *************************************************************************

Cvr2DTexSubPage::Cvr2DTexSubPage(SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec2s & pagesize,
                                 const SbVec2s & texsize)
{
  this->bitspertexel = 0;
  this->clut = NULL;
  this->ispaletted = FALSE;

  assert(pagesize[0] >= 0);
  assert(pagesize[1] >= 0);
  assert(coin_is_power_of_two(pagesize[0]));
  assert(coin_is_power_of_two(pagesize[1]));

  assert(texsize[0] <= pagesize[0]);
  assert(texsize[1] <= pagesize[1]);

  // Actual dimensions of texture bitmap memory block.
  this->texdims = texobj->getDimensions();

  // Calculates part of texture to show.
  this->texmaxcoords = SbVec2f(1.0f, 1.0f);
  if (this->texdims != texsize) {
    this->texmaxcoords[0] = float(texsize[0]) / float(this->texdims[0]);
    this->texmaxcoords[1] = float(texsize[1]) / float(this->texdims[1]);
  }

  // Calculates part of GL quad to show.
  this->quadpartfactors = SbVec2f(1.0f, 1.0f);
  if (pagesize != texsize) {
    this->quadpartfactors[0] = float(texsize[0]) / float(pagesize[0]);
    this->quadpartfactors[1] = float(texsize[1]) / float(pagesize[1]);
  }

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::Cvr2DTexSubPage",
                         "texsize==[%d, %d], "
                         "texobj->getDimensions()==[%d, %d], "
                         "this->texmaxcoords==[%f, %f]",
                         texsize[0], texsize[1],
                         texobj->getDimensions()[0], texobj->getDimensions()[1],
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
    // FIXME: I'm pretty sure this is invoked outside of a GL context
    // from various places in the code. 20030306 mortene.
    GLCMD(glDeleteTextures(1, this->texturename));

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

  if (this->clut) this->clut->unref();
}

SbBool
Cvr2DTexSubPage::isPaletted(void) const
{
  return this->ispaletted;
}

void
Cvr2DTexSubPage::setPalette(const CvrCLUT * newclut)
{
  if (this->clut) { this->clut->unref(); }
  this->clut = newclut;
  newclut->ref();
}

// FIXME: Some magic has to be done to make this one work with OpenGL 1.0.
// torbjorv 08052002
void
Cvr2DTexSubPage::activateTexture(Interpolation interpolation) const
{
  if (this->texturename[0] == 0) {
    GLCMD(glBindTexture(GL_TEXTURE_2D, Cvr2DTexSubPage::emptyimgname[0]));
    return;
  }

  GLCMD(glBindTexture(GL_TEXTURE_2D, this->texturename[0]));

  GLenum interp = 0;
  switch (interpolation) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE); break;
  }

  // FIXME: why are we using float version of glTexParameter here? All
  // arguments looks like ints..? 20031027 mortene.
  GLCMD(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp));
  GLCMD(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp));
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

// Set up the image used (for debugging purposes) when texture memory
// is full.
void
Cvr2DTexSubPage::bindTexMemFullImage(const cc_glglue * glw)
{
  // FIXME: requires > OpenGL 1.0, should go through wrapper.
  GLCMD(glGenTextures(1, Cvr2DTexSubPage::emptyimgname));
  GLCMD(glBindTexture(GL_TEXTURE_2D, Cvr2DTexSubPage::emptyimgname[0]));
  // FIXME: never freed. 20021121 mortene.
    
  // Check format of GIMP-exported "texmem full" image.
  assert(coin_is_power_of_two(tex_image.width));
  assert(coin_is_power_of_two(tex_image.height));
  assert(tex_image.bytes_per_pixel == 4);

  GLCMD(glTexImage2D(GL_TEXTURE_2D,
               0,
               tex_image.bytes_per_pixel,
               tex_image.width, tex_image.height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               tex_image.pixel_data));

  const int texels = tex_image.width * tex_image.height;
  Cvr2DTexSubPage::nroftexels += texels;
  Cvr2DTexSubPage::texmembytes += texels * tex_image.bytes_per_pixel;

  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

  GLCMD(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  GLCMD(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
}

// If no palette specified, this function assumes RGBA data. If a
// palette is specified, the input data should be indices into the
// palette.  The function uses the palette's size to decide whether
// the indices are byte or short.
void
Cvr2DTexSubPage::transferTex2GL(SoGLRenderAction * action,
                                const CvrTextureObject * texobj)
{
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());

  if (Cvr2DTexSubPage::emptyimgname[0] == 0) {
    Cvr2DTexSubPage::bindTexMemFullImage(glw);
  }

  int colorformat;

  this->ispaletted = texobj->getTypeId() == CvrPaletteTexture::getClassTypeId();
  // only knows two types
  assert(this->ispaletted || texobj->getTypeId() == CvrRGBATexture::getClassTypeId());

  // For uploading standard RGBA-texture
  if (!this->ispaletted) {
    colorformat = 4;
    this->bitspertexel = 32; // 8 bits each R, G, B & A
  }
  // For uploading paletted texture
  else {
    colorformat = GL_COLOR_INDEX8_EXT;
    this->bitspertexel = 8;
    if (this->clut) this->clut->unref();
    this->clut = ((CvrPaletteTexture *)texobj)->getCLUT();
    this->clut->ref();
  }

  const int nrtexels = this->texdims[0] * this->texdims[1];
  const int texmem = int(float(nrtexels) * float(this->bitspertexel) / 8.0f);

  // This is a debugging backdoor to test stuff with no limits on how
  // much texture memory we can use.

  // FIXME: there are no restrictions on texture memory yet -- it
  // needs support in other areas of the library. So until proper
  // resource handling is in place, anything goes. 20030324 mortene.

//   static int unlimited_texmem = -1;
  static int unlimited_texmem = 1;
  if (unlimited_texmem == -1) {
    const char * envstr = coin_getenv("CVR_UNLIMITED_TEXMEM");
    if (envstr) { unlimited_texmem = atoi(envstr) > 0 ? 1 : 0; }
    else unlimited_texmem = 0;
  }


  // FIXME: limits should be stored in a global texture manager class
  // or some such. 20021121 mortene.
  if (!unlimited_texmem &&
      (//Cvr2DTexSubPage::detectedtextureswapping ||
       ((nrtexels + Cvr2DTexSubPage::nroftexels) > (16*1024*1024)) ||
       ((texmem + Cvr2DTexSubPage::texmembytes) > (64*1024*1024)))) {
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
    GLCMD(glGenTextures(1, this->texturename));
    assert(glGetError() == GL_NO_ERROR);

    GLCMD(glBindTexture(GL_TEXTURE_2D, this->texturename[0]));
    assert(glGetError() == GL_NO_ERROR);

    void * imgptr = NULL;
    if (this->ispaletted) imgptr = ((CvrPaletteTexture *)texobj)->getIndex8Buffer();
    else imgptr = ((CvrRGBATexture *)texobj)->getRGBABuffer();

    // debugging: keep this around until the peculiar NVidia bug with
    // 1- or 2-pixel width textures has been analyzed.
    //
    // FIXME: need to find out what the hell this problem is. 20031031 mortene.
#if 0
    printf("glTexImage2D() args: dims==<%d,%d>, ispaletted==%d, imgptr==%p\n",
	   this->texdims[0], this->texdims[1], this->ispaletted, imgptr);

    static int allnum = 0;
    allnum++;
    if (this->texdims[0]==1 || this->texdims[1]==1) {
      static int num = 0;
      num++;
      printf("numero %d/%d, nroftexels==%d, texmembytes==%d\n",
	     num, allnum,
	     Cvr2DTexSubPage::nroftexels,
	     Cvr2DTexSubPage::texmembytes);
    }
    else {
      printf("numero %d, nroftexels==%d, texmembytes==%d\n",
	     allnum,
	     Cvr2DTexSubPage::nroftexels,
	     Cvr2DTexSubPage::texmembytes);
    }
#endif // debugging

    GLCMD(glTexImage2D(GL_TEXTURE_2D,
                 0,
                 colorformat,
                 this->texdims[0],
                 this->texdims[1],
                 0,
                 this->ispaletted ? GL_COLOR_INDEX: GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 imgptr));
                 
    assert(glGetError() == GL_NO_ERROR);

    GLint wrapenum = GL_CLAMP;
    // FIXME: investigate if this is really what we want. 20021120 mortene.
    if (cc_glglue_has_texture_edge_clamp(glw)) { wrapenum = GL_CLAMP_TO_EDGE; }

    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapenum));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapenum));
    assert(glGetError() == GL_NO_ERROR);
  }
}

void
Cvr2DTexSubPage::activateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
  this->clut->activate(glw);
}

void
Cvr2DTexSubPage::render(const SoGLRenderAction * action,
                        const SbVec3f & upleft,
                        SbVec3f widthvec, SbVec3f heightvec,
                        Interpolation interpolation)
{
  // Texture binding/activation must happen before setting the
  // palette, or the previous palette will be used.
  this->activateTexture(interpolation);
  if (this->ispaletted) { this->activateCLUT(action); }

  glBegin(GL_QUADS);
  glColor4f(1, 1, 1, 1);

  // Scale span of GL quad to match the visible part of the
  // texture. (Border subpages shouldn't show all of the texture, if
  // the dimensions of the dataset are not a power of two, or if the
  // dimensions are less than the subpage size).

  widthvec *= this->quadpartfactors[0];
  heightvec *= this->quadpartfactors[1];

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

  GLCMD(glEnd());


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
    GLCMD(glDisable(GL_TEXTURE_2D));
    GLCMD(glLineStipple(1, 0xffff));
    GLCMD(glLineWidth(2));

    glBegin(GL_LINE_LOOP);
    glVertex3f(lowleft[0], lowleft[1], lowleft[2]);
    glVertex3f(lowright[0], lowright[1], lowright[2]);
    glVertex3f(upright[0], upright[1], upright[2]);
    glVertex3f(upleft[0], upleft[1], upleft[2]);
    GLCMD(glEnd());

    GLCMD(glEnable(GL_TEXTURE_2D));
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

/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

// This datatype is a color lookup table.

// *************************************************************************

#include <VolumeViz/misc/CvrCLUT.h>

#include <assert.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbBasic.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/elements/CvrPalettedTexturesElement.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrResourceManager.h>

class SoState;

// *************************************************************************

// Fragment program for using an index value to look up a colour from
// a 1D texture.
//
// This is a supplement and an eventual replacement for the palette
// extension. ATI doesn't have the extension, and NVIDIA has
// threatened to remove it from their drivers (and actually did so for
// a few versions before re-introducing it).
//
// Notice that there are some '%s' in the fragment program listing
// below. These will be filled in according to whether they are used
// with 2D or 3D textures, and whether texture contributions should be
// modulated with or replace the current fragment color.

static const char * palettelookupprogram =
"!!ARBfp1.0\n"
"TEMP R0;\n"
"TEX R0.x, fragment.texcoord[0], texture[0], %s;\n"
"TEX R0, R0.x, texture[1], 1D;\n"
"%s;\n"
"END\n";
static const char * palettelookupprogram_modulate =
"MUL result.color, state.material.diffuse, R0";
static const char * palettelookupprogram_replace =
"MOV result.color, R0";

// This is just to have a guaranteed unique pointer value for the
// CvrResourceManager::getInstance() for data which is common for
// CvrCLUT instances (over one GL context).
static const char * CVRCLUT_STATIC_KEYID = "foobar";

// *************************************************************************

// colormap values are between 0 and 255
CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * colormap)
{

  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->int_entries = new uint8_t[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize);

  this->crc32cmap = CvrUtil::crc32(this->int_entries, blocksize);

  this->commonConstructor();
}

// nrcomponents == 1: LUMINANCE
// nrcomponents == 2: LUMINANCE + ALPHA
// nrcomponents == 4: RGBA
//
// values in colormap are between 0.0 and 1.0
CvrCLUT::CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
                 const float * colormap)
{

  this->nrentries = nrcols;
  this->nrcomponents = nrcomponents;
  this->datatype = FLOATS;

  const int blocksize = this->nrentries * this->nrcomponents;
  const int copysize = blocksize * sizeof(float);
  this->flt_entries = new float[blocksize];
  (void)memcpy(this->flt_entries, colormap, copysize);

  this->crc32cmap = CvrUtil::crc32((uint8_t *)this->flt_entries, copysize);

  this->commonConstructor();
}

void
CvrCLUT::commonConstructor(void)
{
  this->refcount = 0;

  this->transparencythresholds[0] = 0;
  this->transparencythresholds[1] = this->nrentries - 1;

  this->alphapolicy = CvrCLUT::ALPHA_AS_IS;

  this->glcolors = new uint8_t[this->nrentries * 4];
  this->regenerateGLColorData();
}

// Copy constructor.
CvrCLUT::CvrCLUT(const CvrCLUT & clut)
{
  this->nrentries = clut.nrentries;
  this->nrcomponents = clut.nrcomponents;
  this->datatype = clut.datatype;

  const int blocksize = this->nrentries * this->nrcomponents;

  switch (this->datatype) {
  case INTS:
    this->int_entries = new uint8_t[blocksize];
    (void)memcpy(this->int_entries, clut.int_entries, blocksize * sizeof(uint8_t));
    break;

  case FLOATS:
    this->flt_entries = new float[blocksize];
    (void)memcpy(this->flt_entries, clut.flt_entries, blocksize * sizeof(float));
    break;

  default: assert(FALSE); break;
  }
  this->crc32cmap = clut.crc32cmap;

  this->refcount = 0;

  this->transparencythresholds[0] = clut.transparencythresholds[0];
  this->transparencythresholds[1] = clut.transparencythresholds[1];

  this->alphapolicy = clut.alphapolicy;

  this->glcolors = new uint8_t[this->nrentries * 4];
  this->regenerateGLColorData();
}

CvrCLUT::~CvrCLUT()
{
  this->killAllGLContextData();

  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;

  delete[] this->glcolors;
}

void
CvrCLUT::killAllGLContextData(void)
{
  this->killAll1DTextures();

  const unsigned int len = (unsigned int)this->contextlist.getLength();
  for (unsigned int i = 0; i < len; i++) {
    struct GLContextStorage * c = this->contextlist[i];
    CvrResourceManager * rm = CvrResourceManager::getInstance(c->ctxid);
    rm->remove(this);
    delete c;
  }
  this->contextlist.truncate(0);
}

// *************************************************************************

void
CvrCLUT::ref(void) const
{
  CvrCLUT * that = (CvrCLUT *)this; // cast away constness
  that->refcount++;
}

void
CvrCLUT::unref(void) const
{
  CvrCLUT * that = (CvrCLUT *)this; // cast away constness
  that->refcount--;
  assert(this->refcount >= 0);
  if (this->refcount == 0) delete this;
}

int32_t
CvrCLUT::getRefCount(void) const
{
  return this->refcount;
}

// *************************************************************************

// Equality comparison between the color lookup tables. The comparison
// will be quick, as it's based on having a checksum for the color
// table.
int
operator==(const CvrCLUT & c1, const CvrCLUT & c2)
{
  if (&c1 == &c2) { return TRUE; }

  if (c1.crc32cmap != c2.crc32cmap) { return FALSE; }

  assert(c1.nrentries == c2.nrentries);
  assert(c1.nrcomponents == c2.nrcomponents);
  assert(c1.datatype == c2.datatype);

#if CVR_DEBUG
  // Checking that CRC32 checksums don't give false positives.  This
  // has implications for performance, so get rid of it after a while.
  const int blocksize = c1.nrentries * c1.nrcomponents;
  const int blksize = blocksize * (c1.datatype == CvrCLUT::INTS ? 1 : sizeof(float));
  assert(memcmp(c1.int_entries, c2.int_entries, blksize) == 0);
#endif // CVR_DEBUG

  if (c1.transparencythresholds[0] != c2.transparencythresholds[0]) { return FALSE; }
  if (c1.transparencythresholds[1] != c2.transparencythresholds[1]) { return FALSE; }
  if (c1.alphapolicy != c2.alphapolicy) { return FALSE; }

  return TRUE;
}

int
operator!=(const CvrCLUT & c1, const CvrCLUT & c2)
{
  return ! operator==(c1, c2);
}

// *************************************************************************

// Everything below "low" (but not including) and above "high" (but
// not including), will be fully transparent.
void
CvrCLUT::setTransparencyThresholds(uint32_t low, uint32_t high)
{
  if ((this->transparencythresholds[0] == low) &&
      (this->transparencythresholds[1] == high)) {
    return;
  }

  assert(low <= high);
  assert(low < this->nrentries);
  assert(high < this->nrentries);

  this->transparencythresholds[0] = low;
  this->transparencythresholds[1] = high;

  this->regenerateGLColorData();
}

void
CvrCLUT::initFragmentProgram(const cc_glglue * glue,
                             CvrCLUT::GlobalGLContextStorage * ctxstorage)
{
  // Two programs, one each for 2D textures and 3D textures.
  cc_glglue_glGenPrograms(glue, 2, ctxstorage->fragmentprogramid);

  for (int i=CvrCLUT::TEXTURE2D; i <= CvrCLUT::TEXTURE3D; i++) {
    cc_glglue_glBindProgram(glue, GL_FRAGMENT_PROGRAM_ARB,
                            ctxstorage->fragmentprogramid[i]);

    // Setup fragment program according to texture type.

    const char * texenvmode =  // Is texture mod. disabled by an envvar?
      CvrUtil::dontModulateTextures() ?
      palettelookupprogram_replace : palettelookupprogram_modulate;

    SbString fragmentprogram;
    fragmentprogram.sprintf(palettelookupprogram,
                            i == CvrCLUT::TEXTURE2D ? "2D" : "3D",
                            texenvmode);

    cc_glglue_glProgramString(glue, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                              fragmentprogram.getLength(), fragmentprogram.getString());

    // FIXME: Maybe a wrapper for catching fragment program errors
    // should be a part of GLUE... (20031204 handegar)
    GLenum err = glGetError();
    if (err) {
      GLint errorPos;
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
      SoDebugError::postWarning("CvrCLUT::initFragmentProgram",
                                "Error in fragment program! (byte pos: %d) '%s'.\n",
                                errorPos, glGetString(GL_PROGRAM_ERROR_STRING_ARB));
    }
  }
}

// Initializes the 1D-texture set up for the fragment program to use
// for palette entry lookups.
void
CvrCLUT::initPaletteTexture(const cc_glglue * glue,
                            CvrCLUT::GLContextStorage * ctxstorage)
{
  assert(ctxstorage->texture1Dclut == 0);

  glGenTextures(1, &ctxstorage->texture1Dclut);

  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, ctxstorage->texture1Dclut);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

  assert(this->nrentries == 256 && "Palette lookup using fragment program will "
         "not work if palette size is != 256");

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, this->nrentries, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, (GLvoid *) this->glcolors);

  // FIXME: shouldn't we restore the glEnable(GL_TEXTURE_1D) here?
  // 20041103 mortene.
}

// *************************************************************************

void
CvrCLUT::contextDeletedCB(void * closure, uint32_t ctxid)
{
  CvrResourceManager * rm = CvrResourceManager::getInstance(ctxid);

  if (closure) { // dynamic (per-CvrCLUT) data
    void * ptr;
    const SbBool ok = rm->get(closure, ptr);
    assert(ok);
    CvrCLUT::GLContextStorage * ctxstorage = (CvrCLUT::GLContextStorage *)ptr;

    if (ctxstorage->texture1Dclut != 0) {
      // FIXME: this resource deletion has not been tested
      // to work yet. 20041111 mortene.
#if CVR_DEBUG && 0 // debug
      SoDebugError::postInfo("CvrCLUT::contextDeletedCB",
                             "deleting 1D-texture %u from GL context %u\n",
                             ctxstorage->texture1Dclut, ctxid);
#endif // debug

      glDeleteTextures(1, &ctxstorage->texture1Dclut);
    }

    rm->remove(closure);
    ((CvrCLUT *)closure)->contextlist.removeItem(ctxstorage);
    delete ctxstorage;
  }
  else { // static/global data for all CvrCLUT instances
    void * ptr;
    const SbBool ok = rm->get(CVRCLUT_STATIC_KEYID, ptr);
    assert(ok);

    CvrCLUT::GlobalGLContextStorage * ctxstorage =
      (CvrCLUT::GlobalGLContextStorage *)ptr;

    if (ctxstorage->fragmentprogramid[0] != 0) {
      // FIXME: this resource deletion has not been tested
      // to work yet. 20041111 mortene.
#if CVR_DEBUG && 0 // debug
      SoDebugError::postInfo("CvrCLUT::contextDeletedCB",
                             "deleting fragment programs %u & %u from "
                             "GL context %u\n",
                             ctxstorage->fragmentprogramid[0],
                             ctxstorage->fragmentprogramid[1],
                             ctxid);
#endif // debug

      const cc_glglue * glw = cc_glglue_instance(ctxid);
      cc_glglue_glDeletePrograms(glw, 2, ctxstorage->fragmentprogramid);
    }

    rm->remove(CVRCLUT_STATIC_KEYID);
    delete ctxstorage;
  }
}

CvrCLUT::GLContextStorage *
CvrCLUT::getGLContextStorage(uint32_t ctxid)
{
  CvrResourceManager * rm = CvrResourceManager::getInstance(ctxid);
  void * ptr;
  const SbBool ok = rm->get(this, ptr);
  if (!ok) {
    ptr = new CvrCLUT::GLContextStorage(ctxid);
    rm->set(this, ptr, CvrCLUT::contextDeletedCB, this);
    CvrCLUT::GLContextStorage * ctxstruct = (CvrCLUT::GLContextStorage *)ptr;
    this->contextlist.append(ctxstruct);
  }

  return (CvrCLUT::GLContextStorage *)ptr;
}

CvrCLUT::GlobalGLContextStorage *
CvrCLUT::getGlobalGLContextStorage(uint32_t ctxid)
{
  CvrResourceManager * rm = CvrResourceManager::getInstance(ctxid);
  void * ptr;
  const SbBool ok = rm->get(CVRCLUT_STATIC_KEYID, ptr);
  if (!ok) {
    ptr = new CvrCLUT::GlobalGLContextStorage;
    rm->set(CVRCLUT_STATIC_KEYID, ptr, CvrCLUT::contextDeletedCB, NULL);
  }

  return (CvrCLUT::GlobalGLContextStorage *)ptr;
}

// *************************************************************************

void
CvrCLUT::activateFragmentProgram(uint32_t ctxid, CvrCLUT::TextureType texturetype) const
{
  const cc_glglue * glw = cc_glglue_instance(ctxid);
  CvrCLUT * thisp = (CvrCLUT *)this;

  CvrCLUT::GLContextStorage * ctxstorage = thisp->getGLContextStorage(ctxid);

  // Shall we generate a new palette texture?
  if (ctxstorage->texture1Dclut == 0) {
    thisp->initPaletteTexture(glw, ctxstorage);
  }

  CvrCLUT::GlobalGLContextStorage * ctxstaticstorage =
    CvrCLUT::getGlobalGLContextStorage(ctxid);

  if (ctxstaticstorage->fragmentprogramid[0] == 0) {
    CvrCLUT::initFragmentProgram(glw, ctxstaticstorage);
  }

  // FIXME: What should we do if unit #1 is already taken? (20040310 handegar)
  cc_glglue_glActiveTexture(glw, GL_TEXTURE1);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, ctxstorage->texture1Dclut);

  cc_glglue_glActiveTexture(glw, GL_TEXTURE0);
  cc_glglue_glBindProgram(glw, GL_FRAGMENT_PROGRAM_ARB,
                          ctxstaticstorage->fragmentprogramid[texturetype]);

  glEnable(GL_FRAGMENT_PROGRAM_ARB);
}

void
CvrCLUT::activatePalette(const cc_glglue * glw, CvrCLUT::TextureType texturetype) const
{
  assert(cc_glglue_has_paletted_textures(glw));

  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  // FIXME: should probably set glColorTableParameter() on
  // GL_COLOR_TABLE_SCALE and GL_COLOR_TABLE_BIAS.

  // FIXME: should probably check if all is ok by using
  // PROXY_TEXTURE_2D first.

  cc_glglue_glColorTable(glw,
                         (texturetype == CvrCLUT::TEXTURE2D) ?
                         GL_TEXTURE_2D : GL_TEXTURE_3D, /* target */
                         GL_RGBA, /* GL internalformat */
                         this->nrentries, /* nr of paletteentries */
                         GL_RGBA, /* palette entry format */
                         GL_UNSIGNED_BYTE, /* palette entry unit type */
                         this->glcolors); /* data ptr */

  // Error checking.

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    static SbBool warn = TRUE;
    if (warn) {
      const char * env = coin_getenv("CVR_NO_GLINT_WARN");
      if (env && atoi(env) > 0) { warn = FALSE; }
    }

    if (warn) {
      warn = FALSE;
      SoDebugError::postWarning("CvrCLUT::activatePalette",
                                "glColorTable(GL_TEXTURE_2D/3D, ...) caused "
                                "glGetError()==0x%x (dec==%d)",
                                err, err);

      // This matches the driver on ASK.trh.sim.no.
      const char * VERSION_GL = "1.1.28 PT";
      const char * VENDOR = "3Dlabs";
      const char * RENDERER = "GLINT R3 PT + GLINT Gamma";
      if (strcmp((const char *)glGetString(GL_VERSION), VERSION_GL) == 0 &&
          strcmp((const char *)glGetString(GL_VENDOR), VENDOR) == 0 &&
          strcmp((const char *)glGetString(GL_RENDERER), RENDERER) == 0) {
        SoDebugError::postWarning("CvrCLUT::activatePalette",
                                  "This is a known problem with this driver "
                                  "(vendor='%s', renderer='%s', version='%s') "
                                  "and seems to be harmless. Turn off this "
                                  "warning by setting the environment "
                                  "variable CVR_NO_GLINT_WARN=1.",
                                  VENDOR, RENDERER, VERSION_GL);
      }
    }
  }

  // Sanity check.
  GLint actualsize;
  cc_glglue_glGetColorTableParameteriv(glw,
                                       (texturetype == CvrCLUT::TEXTURE2D) ?
                                       GL_TEXTURE_2D : GL_TEXTURE_3D,
                                       GL_COLOR_TABLE_WIDTH, &actualsize);

  assert(actualsize == (GLint)this->nrentries);
}

/*!
  Activates the palette we carry, for OpenGL.
*/
void
CvrCLUT::activate(uint32_t ctxid, CvrCLUT::TextureType texturetype) const
{
  const cc_glglue * glw = cc_glglue_instance(ctxid);

  if (CvrCLUT::useFragmentProgramLookup(glw)) {
    this->activateFragmentProgram(ctxid, texturetype);
  }
  else if (CvrCLUT::usePaletteExtension(glw)) {
    this->activatePalette(glw, texturetype);
  }
  else {
    assert(FALSE && "unknown palette activation type");
  }
}

/*!
  Cleans up whatever was done for palette activation.
*/
void
CvrCLUT::deactivate(const cc_glglue * glw) const
{
  if (CvrCLUT::useFragmentProgramLookup(glw)) {
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    cc_glglue_glActiveTexture(glw, GL_TEXTURE1);
    glDisable(GL_TEXTURE_1D);
    cc_glglue_glActiveTexture(glw, GL_TEXTURE0);
  }
}

// *************************************************************************

// Find RGBA color at the given idx.
void
CvrCLUT::lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const
{
  assert(idx < this->nrentries);
  for (int i=0; i < 4; i++) { rgba[i] = this->glcolors[idx * 4 + i]; }
}

// FIXME: this doesn't seem compatible with the fact that
// CvrCLUT-instances should be possible to share between any number of
// textured elements. Must be fixed, or strange errors may
// occur. 20041029 mortene.
void
CvrCLUT::setAlphaUse(AlphaUse policy)
{
  if (this->alphapolicy == policy) { return; }

  this->alphapolicy = policy;
  this->regenerateGLColorData();
}

void
CvrCLUT::regenerateGLColorData(void)
{
  for (unsigned int idx = 0; idx < this->nrentries; idx++) {
    uint8_t * rgba = &this->glcolors[idx * 4];
    if ((idx < this->transparencythresholds[0]) ||
        (idx > this->transparencythresholds[1])) {
      rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0x00;
    }
    else {
      if (this->datatype == FLOATS) {
        const float * colvals = &(this->flt_entries[idx * this->nrcomponents]);
        switch (this->nrcomponents) {
        case 1: // ALPHA
          rgba[0] = rgba[1] = rgba[2] = rgba[3] = uint8_t(colvals[0] * 255.0f);
          break;

        case 2: // LUMINANCE_ALPHA
          rgba[0] = rgba[1] = rgba[2] = uint8_t(colvals[0] * 255.0f);
          rgba[3] = uint8_t(colvals[1] * 255.0f);
          break;

        case 4: // RGBA
          rgba[0] = uint8_t(colvals[0] * 255.0f);
          rgba[1] = uint8_t(colvals[1] * 255.0f);
          rgba[2] = uint8_t(colvals[2] * 255.0f);
          rgba[3] = uint8_t(colvals[3] * 255.0f);
          break;

        default:
          assert(FALSE && "impossible");
          break;
        }
      }
      else if (this->datatype == INTS) {
        const int colidx = idx * 4;
        rgba[0] = this->int_entries[colidx + 0];
        rgba[1] = this->int_entries[colidx + 1];
        rgba[2] = this->int_entries[colidx + 2];
        rgba[3] = this->int_entries[colidx + 3];
      }
      else assert(FALSE);
    }

    switch (this->alphapolicy) {
    case ALPHA_AS_IS: /* "as is", leave alone */ break;
    case ALPHA_OPAQUE: rgba[3] = 0xff; break;
    case ALPHA_BINARY: rgba[3] = (rgba[3] == 0) ? 0 : 0xff; break;
    default: assert(FALSE); break;
    }
  }

  this->killAll1DTextures();
}

// *************************************************************************

void
CvrCLUT::killAll1DTextures(void)
{
  const unsigned int len = (unsigned int)this->contextlist.getLength();
  for (unsigned int i = 0; i < len; i++) {
    struct GLContextStorage * c = this->contextlist[i];
    CvrResourceManager * rm = CvrResourceManager::getInstance(c->ctxid);
    rm->killTexture(c->texture1Dclut);
    c->texture1Dclut = 0;
  }
}

// *************************************************************************

SbBool
CvrCLUT::usePaletteTextures(const SoGLRenderAction * action)
{
  SoState * state = action->getState();
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());

  // Check if paletted textures is wanted by the app programmer.
  const SbBool apiusepalette = CvrPalettedTexturesElement::get(state);
  SbBool usepalettetex = apiusepalette;

  // If we requested paletted textures, does OpenGL support them?
  // Texture paletting can also be done with fragment
  // programs. (E.g. ATI drivers don't have the palette-texture
  // extension, but newer cards supports fragment programs.)
  //
  // (FIXME: one more thing to check versus OpenGL is that the palette
  // size can fit. 2003???? mortene.)
  const SbBool usepaletteextension = CvrCLUT::usePaletteExtension(glw);
  const SbBool usefragmentprogram = CvrCLUT::useFragmentProgramLookup(glw);
  usepalettetex = usepalettetex && (usepaletteextension || usefragmentprogram);

  static SbBool first = TRUE;
  if (first && CvrUtil::doDebugging()) {
    SoDebugError::postInfo("CvrCLUT::usePaletteTextures",
                           "(SoVolumeData::usePalettedTexture==%d, "
                           "use-palette-extension==%d, "
                           "use-fragment-programs==%d) => %s",
                           apiusepalette,
                           usepaletteextension, usefragmentprogram,
                           usepalettetex ? "TRUE" : "FALSE");
    first = FALSE;
  }

  return usepalettetex;
}

SbBool
CvrCLUT::usePaletteExtension(const cc_glglue * glw)
{
  static int disable_palext = -1; // "-1" means "undecided"

  if (disable_palext == -1) {
    const char * env = coin_getenv("CVR_DISABLE_PALETTED_PALEXT");
    disable_palext = env && (atoi(env) > 0);
    if (disable_palext && CvrUtil::doDebugging()) {
      SoDebugError::postInfo("CvrCLUT::usePaletteExtension",
                             "palette extension for paletted textures "
                             "forced OFF");
    }
  }

  if (disable_palext) { return FALSE; }

  return cc_glglue_has_paletted_textures(glw);
}

SbBool
CvrCLUT::useFragmentProgramLookup(const cc_glglue * glw)
{
  static int disable_fragprog = -1; // "-1" means "undecided"

  if (disable_fragprog == -1) {
    const char * env = coin_getenv("CVR_DISABLE_PALETTED_FRAGPROG");
    disable_fragprog = env && (atoi(env) > 0);
    if (disable_fragprog && CvrUtil::doDebugging()) {
      SoDebugError::postInfo("CvrCLUT::useFragmentProgramLookup",
                             "fragment program lookup for paletted textures "
                             "forced OFF");
    }
  }

  if (disable_fragprog) { return FALSE; }

  SbBool usefragmentprogramsupport = cc_glglue_has_arb_fragment_program(glw);
  return usefragmentprogramsupport;
}

// *************************************************************************

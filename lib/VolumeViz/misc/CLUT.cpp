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
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

// This datatype is a color lookup table.

// *************************************************************************

#include <assert.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/SbBasic.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/C/glue/gl.h>

#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>

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

// *************************************************************************

// colormap values are between 0 and 255
CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * colormap)
{

  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;
  this->texturetype = CvrCLUT::TEXTURE2D; // Default

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
  this->texturetype = CvrCLUT::TEXTURE2D; // Default

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

  this->usefragmentprogramlookup = FALSE;
  this->fragmentprograminitialized = FALSE;
  this->palettehaschanged = TRUE;
  this->palettelookuptexture = 0;
  this->texturetype = CvrCLUT::TEXTURE2D; // Default

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
  this->texturetype = clut.texturetype;
  this->usefragmentprogramlookup = clut.usefragmentprogramlookup;
  this->fragmentprograminitialized = clut.fragmentprograminitialized;
  this->palettehaschanged = clut.palettehaschanged;
  this->palettelookuptexture = clut.palettelookuptexture;

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
  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;

  delete[] this->glcolors;
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
CvrCLUT::initFragmentProgram(const cc_glglue * glue)
{
#ifdef HAVE_ARB_FRAGMENT_PROGRAM

  // FIXME: What about mutiple GL contexts (as with
  // soshape_bumpmap.cpp for example)?? Each context would need its
  // own programs. (20040312 handegar)

  cc_glglue_glGenPrograms(glue, 1, &this->palettelookupprogramid);
  cc_glglue_glBindProgram(glue, GL_FRAGMENT_PROGRAM_ARB, this->palettelookupprogramid);

  // Setup fragment program according to texture type.

  const char * texenvmode =  // Is texture mod. disabled by an envvar?
    CvrUtil::dontModulateTextures() ?
    palettelookupprogram_replace : palettelookupprogram_modulate;

  assert(((this->texturetype == CvrCLUT::TEXTURE3D) ||
          (this->texturetype == CvrCLUT::TEXTURE2D)) && "Unknown texture type.");

  SbString fragmentprogram;
  fragmentprogram.sprintf(palettelookupprogram,
                          this->texturetype == CvrCLUT::TEXTURE3D ? "3D" : "2D",
                          texenvmode);

  cc_glglue_glProgramString(glue, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                            fragmentprogram.getLength(), fragmentprogram.getString());

  // FIXME: Maybe a wrapper for catching fragment program errors
  // should be a part of GLUE... (20031204 handegar)
  GLint errorPos;
  GLenum err = glGetError();
  if (err) {
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
    SoDebugError::postWarning("CvrCLUT::initFragmentPrograms",
                              "Error in fragment program! (byte pos: %d) '%s'.\n",
                              errorPos, glGetString(GL_PROGRAM_ERROR_STRING_ARB));
  }

#endif
}

void
CvrCLUT::initPaletteTexture(const cc_glglue * glue)
{
#ifdef HAVE_ARB_FRAGMENT_PROGRAM
  if (this->palettelookuptexture != 0)
    glDeleteTextures(1, &this->palettelookuptexture);

  glGenTextures(1, &this->palettelookuptexture);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, this->palettelookuptexture);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

  assert(this->nrentries == 256 && "Palette lookup using fragment program will "
         "not work if palette size is != 256");

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, this->nrentries, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, (GLvoid *) this->glcolors);
#endif
}

void
CvrCLUT::deactivate(const cc_glglue * glw) const
{
#ifdef HAVE_ARB_FRAGMENT_PROGRAM
  if (!this->usefragmentprogramlookup)
    return;

  glDisable(GL_FRAGMENT_PROGRAM_ARB);
  cc_glglue_glActiveTexture(glw, GL_TEXTURE1);
  glDisable(GL_TEXTURE_1D);
  cc_glglue_glActiveTexture(glw, GL_TEXTURE0);
#endif
}


void
CvrCLUT::activate(const cc_glglue * glw) const
{
#ifdef HAVE_ARB_FRAGMENT_PROGRAM
  // FIXME: We should maybe do a better test than this (Ie. not base
  // the test on glue) (20040310 handegar)
  if (cc_glglue_has_arb_fragment_program(glw)) {

    ((CvrCLUT*)this)->usefragmentprogramlookup = TRUE;

    // Shall we upload a new palette texture?
    if (this->palettehaschanged) {
      // Casting away the const'ness...
      ((CvrCLUT*)this)->initPaletteTexture(glw);
      ((CvrCLUT*)this)->palettehaschanged = FALSE;
    }

    if (!this->fragmentprograminitialized) {
      ((CvrCLUT*)this)->initFragmentProgram(glw);
      ((CvrCLUT*)this)->fragmentprograminitialized = TRUE;
    }

    // FIXME: What should we do if unit #1 is already taken? (20040310 handegar)
    cc_glglue_glActiveTexture(glw, GL_TEXTURE1);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, this->palettelookuptexture);

    cc_glglue_glActiveTexture(glw, GL_TEXTURE0);
    cc_glglue_glBindProgram(glw, GL_FRAGMENT_PROGRAM_ARB, this->palettelookupprogramid);

    glEnable(GL_FRAGMENT_PROGRAM_ARB);

    return;
  }
#endif

  // FIXME: Is this check necessary? It *should* have been done earlier
  // in VoxelChunk.cpp (20040310 handegar)
  if (!cc_glglue_has_paletted_textures(glw)) {
    SoDebugError::postWarning("CvrCLUT::activate",
                              "Trying to use the palette texture extension, but it is not supported.");
    return;
  }

  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  // FIXME: should probably set glColorTableParameter() on
  // GL_COLOR_TABLE_SCALE and GL_COLOR_TABLE_BIAS.

  // FIXME: should probably check if all is ok by using
  // PROXY_TEXTURE_2D first.

  cc_glglue_glColorTable(glw,
                         (this->texturetype == CvrCLUT::TEXTURE2D) ?
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
      SoDebugError::postWarning("CvrCLUT::activate",
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
        SoDebugError::postWarning("CvrCLUT::activate",
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
                                       (this->texturetype == CvrCLUT::TEXTURE2D) ?
                                       GL_TEXTURE_2D : GL_TEXTURE_3D,
                                       GL_COLOR_TABLE_WIDTH, &actualsize);

  assert(actualsize == (GLint)this->nrentries);
}

// Find RGBA color at the given idx.
void
CvrCLUT::lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const
{
  assert(idx < this->nrentries);
  for (int i=0; i < 4; i++) { rgba[i] = this->glcolors[idx * 4 + i]; }
}

void
CvrCLUT::setAlphaUse(AlphaUse policy)
{
  if (this->alphapolicy == policy) { return; }

  this->alphapolicy = policy;
  this->regenerateGLColorData();
}

void
CvrCLUT::setTextureType(TextureType type)
{
  this->texturetype = type;
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

  this->palettehaschanged = TRUE;
}

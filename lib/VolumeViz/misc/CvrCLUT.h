#ifndef SIMVOLEON_CVRCLUT_H
#define SIMVOLEON_CVRCLUT_H

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

#include <Inventor/SbBasic.h>
#include <Inventor/system/gl.h>

struct cc_glglue;
class SoGLRenderAction;

// *************************************************************************

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrcols, const uint8_t * colormap);
  CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
          const float * colormap);
  CvrCLUT(const CvrCLUT & clut);

  friend int operator==(const CvrCLUT & c1, const CvrCLUT & c2);
  friend int operator!=(const CvrCLUT & c1, const CvrCLUT & c2);

  void ref(void) const;
  void unref(void) const;
  int32_t getRefCount(void) const;

  void setTransparencyThresholds(uint32_t low, uint32_t high);

  enum TextureType { TEXTURE2D = 0, TEXTURE3D = 1 };

  void activate(const cc_glglue * glw, TextureType t) const;
  void deactivate(const cc_glglue * glw) const;

  void lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const;

  // Note: must match the enum in SoOrthoSlice.
  enum AlphaUse { ALPHA_AS_IS = 0, ALPHA_OPAQUE = 1, ALPHA_BINARY = 2 };

  void setAlphaUse(AlphaUse policy);

  static SbBool usePaletteTextures(const SoGLRenderAction * action);

private:
  ~CvrCLUT();
  void commonConstructor(void);
  void regenerateGLColorData(void);
  static void initFragmentProgram(const cc_glglue * glue);
  void initPaletteTexture(const cc_glglue * glue);

  unsigned int nrentries;
  unsigned int nrcomponents;


  enum DataType { INTS, FLOATS } datatype;
  union {
    uint8_t * int_entries;
    float * flt_entries;
  };
  uint32_t crc32cmap;

  uint32_t transparencythresholds[2];
  AlphaUse alphapolicy;

  uint8_t * glcolors;

  SbBool palettehaschanged;
  SbBool usefragmentprogramlookup;
  GLuint palettelookuptexture;
  static GLuint fragmentprogramid[2];

  int refcount;
  friend class nop; // to avoid g++ compiler warning on the private constructor
};

// *************************************************************************

int operator==(const CvrCLUT & c1, const CvrCLUT & c2);
int operator!=(const CvrCLUT & c1, const CvrCLUT & c2);

// *************************************************************************

#endif // !SIMVOLEON_CVRCLUT_H

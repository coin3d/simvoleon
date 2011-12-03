#ifndef SIMVOLEON_CVRCLUT_H
#define SIMVOLEON_CVRCLUT_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <Inventor/SbBasic.h>
#include <Inventor/system/gl.h>
#include <Inventor/lists/SbList.h>

struct cc_glglue;
class SoGLRenderAction;

// *************************************************************************

class CvrCLUT {
public:
  // Note: must match the enum in SoOrthoSlice.
  enum AlphaUse { ALPHA_AS_IS = 0, ALPHA_OPAQUE = 1, ALPHA_BINARY = 2 };

  CvrCLUT(const unsigned int nrcols, const uint8_t * colormap, AlphaUse policy);
  CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
          const float * colormap, AlphaUse policy);
  CvrCLUT(const CvrCLUT & clut);

  friend int operator==(const CvrCLUT & c1, const CvrCLUT & c2);
  friend int operator!=(const CvrCLUT & c1, const CvrCLUT & c2);

  void ref(void) const;
  void unref(void) const;
  int32_t getRefCount(void) const;

  void setTransparencyThresholds(uint32_t low, uint32_t high);

  enum TextureType { TEXTURE2D = 0, TEXTURE3D = 1, TEXTURE3D_GRADIENT = 2 };

  void activate(uint32_t ctxid, TextureType t) const;
  void deactivate(const cc_glglue * glw) const;

  void lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const;

  static SbBool usePaletteTextures(const SoGLRenderAction * action);

  static SbBool usePaletteExtension(const cc_glglue * glw);
  static SbBool useFragmentProgramLookup(const cc_glglue * glw);

private:
  ~CvrCLUT();
  void commonConstructor(AlphaUse policy);
  void regenerateGLColorData(void);

  void setAlphaUse(AlphaUse policy);

  struct GLContextStorage {
    GLContextStorage(uint32_t id)
    {
      this->texture1Dclut = 0;
      this->ctxid = id;
    }
    
    GLuint texture1Dclut;
    uint32_t ctxid;
  };
  SbList<struct GLContextStorage *> contextlist;
  struct GlobalGLContextStorage {
    GlobalGLContextStorage(void)
    {
      this->fragmentprogramid[0] = this->fragmentprogramid[1] =
        this->fragmentprogramid[2] = 0;
    }

    GLuint fragmentprogramid[3];
  };
  static GlobalGLContextStorage * getGlobalGLContextStorage(uint32_t ctxid);
  GLContextStorage * getGLContextStorage(uint32_t ctxid);
  static void contextDeletedCB(void * closure, uint32_t contextid);

  void killAll1DTextures(void);
  void killAllGLContextData(void);

  static void initFragmentProgram(const cc_glglue * glue, GlobalGLContextStorage * ctxstorage);
  void initPaletteTexture(const cc_glglue * glue, GLContextStorage * ctxstorage);
  void activateFragmentProgram(uint32_t ctxid, CvrCLUT::TextureType t) const;
  void activatePalette(const cc_glglue * glw, CvrCLUT::TextureType t) const;

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

  int refcount;

  friend class nop; // to avoid g++ compiler warning on the private constructor
};

// *************************************************************************

int operator==(const CvrCLUT & c1, const CvrCLUT & c2);
int operator!=(const CvrCLUT & c1, const CvrCLUT & c2);

// *************************************************************************

#endif // !SIMVOLEON_CVRCLUT_H

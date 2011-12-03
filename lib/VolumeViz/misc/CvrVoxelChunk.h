#ifndef SIMVOLEON_CVRVOXELCHUNK_H
#define SIMVOLEON_CVRVOXELCHUNK_H

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

#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3s.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/misc/CvrCLUT.h>

class CvrTextureObject;
class SoGLRenderAction;
class SoTransferFunctionElement;
class SbBox2s;

// *************************************************************************

class CvrVoxelChunk {
public:
  CvrVoxelChunk(const SbVec3s & dimensions, unsigned int bytesprvoxel,
                const void * buffer = NULL);
  ~CvrVoxelChunk();

  void transfer(const SoGLRenderAction * action, const CvrCLUT * clut, CvrTextureObject * texobj, SbBool & invisible) const;

  const void * getBuffer(void) const;
  const uint8_t * getBuffer8(void) const;
  const uint16_t * getBuffer16(void) const;

  unsigned int bufferSize(void) const;

  const SbVec3s & getDimensions(void) const;
  unsigned int getUnitSize(void) const;

  void dumpToPPM(const char * filename) const;

  // FIXME: move to CvrCLUT?
  static CvrCLUT * getCLUT(const SoTransferFunctionElement * e, CvrCLUT::AlphaUse alphause);

  CvrVoxelChunk * buildSubPage(const unsigned int axisidx, const int pageidx,
                               const SbBox2s & cutslice);

  CvrVoxelChunk * buildSubCube(const SbBox3s & cubecut);

private:
  void transfer2D(const SoGLRenderAction * action, const CvrCLUT * clut, CvrTextureObject * texobj, SbBool & invisible) const;
  void transfer3D(const SoGLRenderAction * action, const CvrCLUT * clut, CvrTextureObject * texobj, SbBool & invisible) const;
  
  CvrVoxelChunk * buildSubPageX(const int pageidx, const SbBox2s & cutslice);
  CvrVoxelChunk * buildSubPageY(const int pageidx, const SbBox2s & cutslice);
  CvrVoxelChunk * buildSubPageZ(const int pageidx, const SbBox2s & cutslice);

  static CvrCLUT * makeCLUT(const SoTransferFunctionElement * e, CvrCLUT::AlphaUse alphause);
  static SbDict * CLUTdict;

  static uint8_t PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];
  static void initPredefGradients(void);

  SbBool destructbuffer;
  const void * voxelbuffer;
  SbVec3s dimensions;
  unsigned int unitsize;
};

// *************************************************************************

#endif // !SIMVOLEON_CVRVOXELCHUNK_H

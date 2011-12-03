#ifndef SIMVOLEON_CVRPAGEHANDLER_H
#define SIMVOLEON_CVRPAGEHANDLER_H

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

#include <Inventor/SbBox2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/misc/CvrCLUT.h>

class Cvr2DTexPage;
class SoVolumeData;
class SoState;
class SoGLRenderAction;

// *************************************************************************

class CvrPageHandler {
public:
  CvrPageHandler(const SoGLRenderAction * action);
  ~CvrPageHandler();

  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };

  void render(const SoGLRenderAction * action, CvrCLUT::AlphaUse alphause, unsigned int numslices,
              // FIXME: composition should be passed on state
              // stack. 20040722 mortene.
              CvrPageHandler::Composition composition,
              // FIXME: abort-callback, and the numslices argument
              // above, should both be passed on the state
              // stack. 20040722 mortene.
              SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
              void * abortcbdata);

  unsigned int getCurrentAxis(const SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(const SoGLRenderAction * action, SbVec3f & direction) const;
  Cvr2DTexPage * getSlice(const SoGLRenderAction * action,
                          const unsigned int AXISIDX, unsigned int sliceidx);

  // FIXME: should rather use an extension Coin cache, than using
  // comparison functions and storing the subpagesize
  // locally. 20040722 mortene.
  void comparePageSize(const SbVec3s & currsubpagesize);
  SbVec3s subpagesize;

  void setPalette(const CvrCLUT * c);

  Cvr2DTexPage ** slices[3];
  unsigned int voldatadims[3];

  uint32_t transferfuncid;
  uint32_t voxelblockelementnodeid;
  const CvrCLUT * clut;
};

#endif // !SIMVOLEON_CVRPAGEHANDLER_H

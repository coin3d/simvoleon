#ifndef SIMVOLEON_CVRCUBEHANDLER_H
#define SIMVOLEON_CVRCUBEHANDLER_H

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

#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoObliqueSlice.h>
#include <VolumeViz/misc/CvrCLUT.h>

// *************************************************************************

class Cvr3DTexCube;
class SoVolumeReader;
class SoVolumeData;
class SoState;
class SoGLRenderAction;
class SbVec3f;

// *************************************************************************

class CvrCubeHandler {
public:
  CvrCubeHandler(void);
  ~CvrCubeHandler();

  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };

  void render(SoGLRenderAction * action, CvrCLUT::AlphaUse alphause, unsigned int numslices,
              CvrCubeHandler::Composition composition,
              SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
              void * abortcbdata);

  void renderObliqueSlice(SoGLRenderAction * action,
                          SoObliqueSlice::AlphaUse alphause,
                          const SbPlane plane);

  unsigned int getCurrentAxis(SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(SoGLRenderAction * action, SbVec3f & direction) const;
  void setPalette(const CvrCLUT * c);

  Cvr3DTexCube * volumecube;
  const CvrCLUT * clut;
  SbUniqueId voxelblockelementnodeid;
  
  SbBool lighting;
  SbVec3f lightDirection;
  float lightIntensity;

};

#endif // !SIMVOLEON_CVRCUBEHANDLER_H

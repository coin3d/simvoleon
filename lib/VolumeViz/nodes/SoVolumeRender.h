#ifndef COIN_SOVOLUMERENDER_H
#define COIN_SOVOLUMERENDER_H

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

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoVolumeRender : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoVolumeRender);

public:
  static void initClass(void);
  SoVolumeRender(void);

  enum Interpolation { NEAREST, LINEAR };
  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };
  enum NumSlicesControl { ALL, MANUAL, AUTOMATIC };

  enum AbortCode { CONTINUE, ABORT, SKIP };
  typedef AbortCode SoVolumeRenderAbortCB(int totalslices, int thisslice, 
                                          void * userdata);

  void setAbortCallback(SoVolumeRenderAbortCB * func, void * userdata = NULL);

  SoSFEnum interpolation;
  SoSFEnum composition;
  SoSFBool lighting;
  SoSFVec3f lightDirection;
  SoSFFloat lightIntensity;
  SoSFEnum numSlicesControl;
  SoSFInt32 numSlices;
  SoSFBool viewAlignedSlices;

protected:
  ~SoVolumeRender();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:  
  friend class SoVolumeRenderP;
  class SoVolumeRenderP * pimpl;
};

#endif // !COIN_SOVOLUMERENDER_H

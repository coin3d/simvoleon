#ifndef COIN_SOROI_H
#define COIN_SOROI_H

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

// FIXME: class not yet properly implemented. 20040505 mortene.


#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoSFBox3s.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFEnum.h>


class SIMVOLEON_DLL_API SoROI : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoROI);

public:
  SoROI(void);
  static void initClass(void);

  enum Flags {
    ENABLE_X0 = 0x1,
    ENABLE_Y0 = 0x2,
    ENABLE_Z0 = 0x4,
    INVERT_0 = 0x8,
    ENABLE_X1 = 0x10,
    ENABLE_Y1 = 0x20,
    ENABLE_Z1 = 0x40,
    INVERT_1 = 0x80,
    ENABLE_X2 = 0x100,
    ENABLE_Y2 = 0x200,
    ENABLE_Z2 = 0x400,
    INVERT_2 = 0x800,
    OR_SELECT = 0x1000,
    INVERT_OUTPUT = 0x2000,
    SUB_VOLUME = ENABLE_X0 | ENABLE_Y0 | ENABLE_Z0,
    EXCLUSION_BOX = SUB_VOLUME | INVERT_OUTPUT,
    CROSS = ENABLE_X0 | ENABLE_Y0 | ENABLE_Y1 | ENABLE_Z1 | ENABLE_X2 | ENABLE_Z2 | OR_SELECT,
    CROSS_INVERT = CROSS | INVERT_OUTPUT,
    FENCE = ENABLE_X0 | ENABLE_Y1 | ENABLE_Z2 | OR_SELECT,
    FENCE_INVERT = FENCE | INVERT_OUTPUT
  };

  SoSFBox3s box;
  SoSFEnum flags;
  SoSFBox3s subVolume;
  SoSFBool relative;

protected:
  ~SoROI();

  virtual void GLRender(SoGLRenderAction * action);

  // FIXME: Implement these functions... torbjorv 07312002
  virtual void doAction(SoAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void pick(SoPickAction * action);

private:
  friend class SoROIP;
  class SoROIP * pimpl;
};

#endif // !COIN_SOROI_H

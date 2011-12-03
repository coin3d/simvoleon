#ifndef COIN_SOTRANSFERFUNCTION_H
#define COIN_SOTRANSFERFUNCTION_H

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

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFUInt32.h>

// *************************************************************************

class SIMVOLEON_DLL_API SoTransferFunction : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoTransferFunction);

public:
  static void initClass(void);
  SoTransferFunction(void);

  enum PredefColorMap {
    NONE = 0,
    GREY,
    GRAY = GREY,
    TEMPERATURE,
    PHYSICS,
    STANDARD,
    GLOW,
    BLUE_RED,
    SEISMIC
  };

  enum ColorMapType {
    ALPHA,
    LUM_ALPHA,
    RGBA
  };

  SoSFInt32 shift;
  SoSFInt32 offset;
  SoSFEnum predefColorMap;
  SoSFEnum colorMapType;
  SoMFFloat colorMap;

  void reMap(int low, int high);
  SbBool hasTransparency(void) const;

protected:
  ~SoTransferFunction();

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void pick(SoPickAction * action);

private:
  // These were added to make it possible to control the narrowing of
  // the transfer function from the iv-file. They provide the same
  // functionality as the reMap() function.
  //
  // These are "unofficial", private fields, as they are not available
  // in TGS VolumeViz, which we want to stay compatible with.
  SoSFUInt32 remapLow;
  SoSFUInt32 remapHigh;

  friend class SoTransferFunctionP;
  class SoTransferFunctionP * pimpl;
};

// *************************************************************************

#endif // !COIN_SOTRANSFERFUNCTION_H

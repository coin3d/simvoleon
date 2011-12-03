#ifndef SIMVOLEON_CVR2DTEXSUBPAGE_H
#define SIMVOLEON_CVR2DTEXSUBPAGE_H

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

#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/C/glue/gl.h>

class SoGLRenderAction;
class CvrTextureObject;
class CvrCLUT;


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(const SoGLRenderAction * action,
                  const CvrTextureObject * texobj,
                  const SbVec2s & pagesize, 
                  const SbVec2s & texsize);
  ~Cvr2DTexSubPage();

  void render(const SoGLRenderAction * action,
              const SbVec3f & upleft, SbVec3f widthvec, SbVec3f heightvec);

  SbBool isPaletted(void) const;

  // FIXME: this should just be picked up from the state stack, and
  // handled by the CvrTextureObject subclasses. 20040721 mortene.
  void setPalette(const CvrCLUT * newclut);

private:
  void renderQuad(const SoGLRenderAction * action,
                  const SbVec3f & upleft, const SbVec3f & lowleft,
                  const SbVec3f & upright, const SbVec3f & lowright);

  void activateCLUT(const SoGLRenderAction * action);
  void deactivateCLUT(const SoGLRenderAction * action);

  static void bindTexMemFullImage(const cc_glglue * glw);

  static GLuint emptyimgname[1];
  SbVec2f texmaxcoords;
  SbVec2f quadpartfactors;
  unsigned int bitspertexel;
  const CvrCLUT * clut;
  const CvrTextureObject * texobj;
};


#endif // !SIMVOLEON_CVR2DTEXSUBPAGE_H

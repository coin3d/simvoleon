#ifndef COIN_CVR2DTEXSUBPAGE_H
#define COIN_CVR2DTEXSUBPAGE_H

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

#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/C/glue/gl.h>

class SoGLRenderAction;
class CvrTextureObject;
class CvrCLUT;


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(SoGLRenderAction * action,
                  const CvrTextureObject * texobj,
                  const SbVec2s & pagesize, 
                  const SbVec2s & texsize,
                  const SbBool compresstextures);
  ~Cvr2DTexSubPage();

  enum Interpolation { NEAREST, LINEAR };

  void render(const SoGLRenderAction * action,
              const SbVec3f & upleft, SbVec3f widthvec, SbVec3f heightvec,
              Interpolation interpolation);

  SbBool isPaletted(void) const;
  void setPalette(const CvrCLUT * newclut);

  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

private:
  void transferTex2GL(SoGLRenderAction * action,
                      const CvrTextureObject * texobj);

  void activateTexture(Interpolation interpolation) const;
  void activateCLUT(const SoGLRenderAction * action);
  void deactivateCLUT(const SoGLRenderAction * action);

  static void bindTexMemFullImage(const cc_glglue * glw);

  GLuint texturename[1];
  static GLuint emptyimgname[1];
  SbVec2s texdims;
  SbVec2f texmaxcoords;
  SbVec2f quadpartfactors;
  static unsigned int nroftexels;
  static unsigned int texmembytes;
  static SbBool detectedtextureswapping;
  unsigned int bitspertexel;
  const CvrCLUT * clut;
  SbBool ispaletted;
  SbBool compresstextures;
};


#endif //COIN_CVR2DTEXSUBPAGE_H

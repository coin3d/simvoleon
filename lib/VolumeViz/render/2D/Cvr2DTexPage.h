#ifndef SIMVOLEON_CVR2DTEXPAGE_H
#define SIMVOLEON_CVR2DTEXPAGE_H

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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <Inventor/SbVec2s.h>

class Cvr2DTexSubPage;
class CvrCLUT;
class SbVec3f;
class SoGLRenderAction;
class SoState;

// *************************************************************************

class Cvr2DTexPage {

public:
  Cvr2DTexPage(const SoGLRenderAction * action,
               const unsigned int axis, const unsigned int sliceidx,
               const SbVec2s & subpagetexsize);
  ~Cvr2DTexPage();

  void render(const SoGLRenderAction * action, const SbVec3f & origo,
              const SbVec3f & horizspan, const SbVec3f & verticalspan);

  void setPalette(const CvrCLUT * c);
  const CvrCLUT * getPalette(void) const;

private:
  class Cvr2DTexSubPageItem * getSubPage(SoState * state, int col, int row);

  class Cvr2DTexSubPageItem * buildSubPage(const SoGLRenderAction * action,
                                           int col, int row);

  void releaseSubPage(Cvr2DTexSubPage * page);

  void releaseAllSubPages(void);
  void releaseSubPage(const int row, const int col);

  int calcSubPageIdx(int row, int col) const;

  class Cvr2DTexSubPageItem ** subpages;

  unsigned int axis;
  unsigned int sliceidx;
  SbVec2s subpagesize;
  SbVec2s dimensions;

  int nrcolumns;
  int nrrows;

  const CvrCLUT * clut;
};

#endif // !SIMVOLEON_CVR2DTEXPAGE_H

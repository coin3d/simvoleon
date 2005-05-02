#ifndef SIMVOLEON_CVRLIGHTINGELEMENT_H
#define SIMVOLEON_CVRLIGHTINGELEMENT_H

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

#include <Inventor/SbVec3f.h>
#include <Inventor/elements/SoSubElement.h>

// *************************************************************************

class CvrLightingElement : public SoElement {
  typedef SoElement inherited;

  SO_ELEMENT_HEADER(CvrLightingElement);

public:
  static void initClass(void);
  virtual void init(SoState * state);
  static const CvrLightingElement * getInstance(SoState * const state);

  virtual SbBool matches(const SoElement * element) const;
  virtual SoElement * copyMatchInfo(void) const;

  static void set(SoState * state, const SbBool & lighting, const SbVec3f & lightDirection, const float & lightIntensity);
  static SbBool useLighting(SoState * state);
  static void get(SoState * state, SbVec3f & lightDirection, float & lightIntensity);

protected:
  virtual ~CvrLightingElement();

private:
  SbBool lighting;
  SbVec3f lightDirection;
  float lightIntensity;
};

// *************************************************************************

#endif // !SIMVOLEON_CVRLIGHTINGELEMENT_H

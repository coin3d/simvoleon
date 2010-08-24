#ifndef SIMVOLEON_CVRDATASIZEELEMENT_H
#define SIMVOLEON_CVRDATASIZEELEMENT_H

/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#include <Inventor/SbVec3s.h>
#include <Inventor/elements/SoSubElement.h>

// *************************************************************************

class CvrDataSizeElement : public SoElement {
  typedef SoElement inherited;

  SO_ELEMENT_HEADER(CvrDataSizeElement);

public:
  static void initClass(void);
  virtual void init(SoState * state);
  static const CvrDataSizeElement * getInstance(SoState * const state);

  virtual SbBool matches(const SoElement * element) const;
  virtual SoElement * copyMatchInfo(void) const;

  static void set(SoState * state, const SbVec3s & value);
  static SbVec3s get(SoState * state);

protected:
  virtual ~CvrDataSizeElement();

private:
  SbVec3s size;
};

// *************************************************************************

#endif // !SIMVOLEON_CVRDATASIZEELEMENT_H

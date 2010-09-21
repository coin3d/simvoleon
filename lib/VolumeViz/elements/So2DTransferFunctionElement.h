#ifndef COIN_SO2DTRANSFERFUNCTIONELEMENT_H
#define COIN_SO2DTRANSFERFUNCTIONELEMENT_H

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

#include <Inventor/elements/SoReplacedElement.h>
#include <VolumeViz/C/basic.h>

class So2DTransferFunction;

// *************************************************************************

class So2DTransferFunctionElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(So2DTransferFunctionElement);

public:
  static void initClass(void);
  virtual void init(SoState * state);
  static void setTransferFunction(SoState * const state, So2DTransferFunction * node);
  So2DTransferFunction * getTransferFunction(void) const;
  static const So2DTransferFunctionElement * getInstance(SoState * const state);

protected:
  virtual ~So2DTransferFunctionElement();

private:
  So2DTransferFunction * transferfunction;
};

#endif // !COIN_SO2DTRANSFERFUNCTIONELEMENT_H

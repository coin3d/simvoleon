#ifndef COIN_SOTRANSFERFUNCTIONELEMENT_H
#define COIN_SOTRANSFERFUNCTIONELEMENT_H

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

#include <Inventor/elements/SoReplacedElement.h>
#include <VolumeViz/C/basic.h>

class SoTransferFunction;

// *************************************************************************

// Note: no longer part of public API, was taken out for SIM Voleon
// 2.0.0. If requested, we could easily add it back again.

class SoTransferFunctionElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;
  SO_ELEMENT_HEADER(SoTransferFunctionElement);

public:
  static void initClass(void);
  virtual void init(SoState * state);


  static void setTransferFunction(SoState * const state,
                                  SoTransferFunction * node);
  SoTransferFunction * getTransferFunction(void) const;


  static void setTransparencyThresholds(SoState * const state,
                                        uint32_t low, uint32_t high);
  void getTransparencyThresholds(uint32_t & low, uint32_t & high) const;


  static const SoTransferFunctionElement * getInstance(SoState * const state);


protected:
  virtual ~SoTransferFunctionElement();

private:
  SoTransferFunction * transferfunction;
  uint32_t transpthreshold[2];
};

#endif // !COIN_SOTRANSFERFUNCTIONELEMENT_H

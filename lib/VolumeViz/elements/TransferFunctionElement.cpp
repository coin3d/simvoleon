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

/*!
  \class SoTransferFunctionElement VolumeViz/elements/SoTransferFunctionElement.h
  \brief This class stores a reference to the current transfer function.
*/

// *************************************************************************

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <assert.h>

// *************************************************************************

SO_ELEMENT_SOURCE(SoTransferFunctionElement);

// *************************************************************************

void
SoTransferFunctionElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(SoTransferFunctionElement, inherited);
}


SoTransferFunctionElement::~SoTransferFunctionElement(void)
{
}

void
SoTransferFunctionElement::init(SoState * state)
{
  inherited::init(state);

  this->transferfunction = NULL;

  // Init to lowest and highest uint32_t values.  The setting of the
  // max value seems a bit tricky, but it's done like that to avoid
  // overflow.
  this->transpthreshold[0] = 0;
  this->transpthreshold[1] = (uint32_t(1 << 31) - 1) * 2 + 1;
}

void
SoTransferFunctionElement::setTransferFunction(SoState * const state,
                                               SoTransferFunction * node)
{
  SoTransferFunctionElement * elem = (SoTransferFunctionElement *)
    SoElement::getElement(state, SoTransferFunctionElement::classStackIndex);

  if (elem) { elem->transferfunction = node; }
}


SoTransferFunction *
SoTransferFunctionElement::getTransferFunction(void) const
{
  return this->transferfunction;
}

void
SoTransferFunctionElement::setTransparencyThresholds(SoState * const state,
                                                     uint32_t low,
                                                     uint32_t high)
{
  SoTransferFunctionElement * elem = (SoTransferFunctionElement *)
    SoElement::getElement(state, SoTransferFunctionElement::classStackIndex);

  if (elem) {
    elem->transpthreshold[0] = low;
    elem->transpthreshold[1] = high;
  }
}

void
SoTransferFunctionElement::getTransparencyThresholds(uint32_t & low,
                                                     uint32_t & high) const
{
  low = this->transpthreshold[0];
  high = this->transpthreshold[1];
}

const SoTransferFunctionElement *
SoTransferFunctionElement::getInstance(SoState * const state)
{
  return (const SoTransferFunctionElement *)
    SoTransferFunctionElement::getConstElement(state, SoTransferFunctionElement::classStackIndex);
}

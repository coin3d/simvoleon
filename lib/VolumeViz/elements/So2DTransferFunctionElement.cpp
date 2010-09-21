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


/*!
  \class So2DTransferFunctionElement VolumeViz/elements/So2DTransferFunctionElement.h
  \brief This class stores a reference to the current transfer function.
*/

// *************************************************************************

#include <VolumeViz/elements/So2DTransferFunctionElement.h>

// *************************************************************************

SO_ELEMENT_SOURCE(So2DTransferFunctionElement);

// *************************************************************************

void
So2DTransferFunctionElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(So2DTransferFunctionElement, inherited);
}


So2DTransferFunctionElement::~So2DTransferFunctionElement(void)
{
}


void
So2DTransferFunctionElement::init(SoState * state)
{
  inherited::init(state);
  this->transferfunction = NULL;
}

void
So2DTransferFunctionElement::setTransferFunction(SoState * const state,
                                                 So2DTransferFunction * node)
{
  So2DTransferFunctionElement * elem = (So2DTransferFunctionElement *)
    SoElement::getElement(state, So2DTransferFunctionElement::classStackIndex);
  
  if (elem) { 
    elem->transferfunction = node; 
  }
}


So2DTransferFunction *
So2DTransferFunctionElement::getTransferFunction(void) const
{
  return this->transferfunction;
}


const So2DTransferFunctionElement *
So2DTransferFunctionElement::getInstance(SoState * const state)
{
  return (const So2DTransferFunctionElement *)
    So2DTransferFunctionElement::getConstElement(state, So2DTransferFunctionElement::classStackIndex);
}

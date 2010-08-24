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

#include <VolumeViz/elements/CvrDataSizeElement.h>

#include <assert.h>

// *************************************************************************

SO_ELEMENT_SOURCE(CvrDataSizeElement);

// *************************************************************************

void
CvrDataSizeElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(CvrDataSizeElement, inherited);
}


CvrDataSizeElement::~CvrDataSizeElement(void)
{
}

void
CvrDataSizeElement::init(SoState * state)
{
  inherited::init(state);
  this->size.setValue(0, 0, 0);
}

const CvrDataSizeElement *
CvrDataSizeElement::getInstance(SoState * const state)
{
  return (const CvrDataSizeElement *)
    CvrDataSizeElement::getConstElement(state,
                                        CvrDataSizeElement::classStackIndex);
}

// *************************************************************************

SbBool
CvrDataSizeElement::matches(const SoElement * element) const
{
  return
    inherited::matches(element) &&
    ((CvrDataSizeElement *)element)->size == this->size;
}

SoElement *
CvrDataSizeElement::copyMatchInfo(void) const
{
  assert(this->getTypeId().canCreateInstance());

  CvrDataSizeElement * element = (CvrDataSizeElement *)
    this->getTypeId().createInstance();

  *element = *this;

  return element;
}


void
CvrDataSizeElement::set(SoState * state, const SbVec3s & value)
{
  CvrDataSizeElement * element = (CvrDataSizeElement *)
    CvrDataSizeElement::getElement(state, CvrDataSizeElement::classStackIndex);
  element->size = value;
}

SbVec3s
CvrDataSizeElement::get(SoState * state)
{
  const CvrDataSizeElement * element = (const CvrDataSizeElement *)
    CvrDataSizeElement::getConstElement(state, CvrDataSizeElement::classStackIndex);
  return element->size;
}

// *************************************************************************

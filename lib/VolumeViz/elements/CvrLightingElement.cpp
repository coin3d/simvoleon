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

#include <VolumeViz/elements/CvrLightingElement.h>

#include <assert.h>

// *************************************************************************

SO_ELEMENT_SOURCE(CvrLightingElement);

// *************************************************************************

void
CvrLightingElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(CvrLightingElement, inherited);
}


CvrLightingElement::~CvrLightingElement(void)
{
}

void
CvrLightingElement::init(SoState * state)
{
  inherited::init(state);
  this->lighting = false;
  this->lightDirection = SbVec3f(-1, -1, -1);
  this->lightIntensity = 1.0f;
}

const CvrLightingElement *
CvrLightingElement::getInstance(SoState * const state)
{
  return (const CvrLightingElement *)
    CvrLightingElement::getConstElement(state,
                                        CvrLightingElement::classStackIndex);
}

// *************************************************************************

SbBool
CvrLightingElement::matches(const SoElement * element) const
{
  return ((CvrLightingElement *)element)->lighting == this->lighting;
}

SoElement *
CvrLightingElement::copyMatchInfo(void) const
{
  assert(this->getTypeId().canCreateInstance());

  CvrLightingElement * element = (CvrLightingElement *)
    this->getTypeId().createInstance();

  element->lighting = lighting;

  return element;
}


void
CvrLightingElement::set(SoState * state, const SbBool & lighting, const SbVec3f & lightDirection, const float & lightIntensity)
{
  CvrLightingElement * element = (CvrLightingElement *)
    CvrLightingElement::getElement(state, CvrLightingElement::classStackIndex);
  element->lighting = lighting;
  element->lightDirection = lightDirection;
  element->lightIntensity = lightIntensity;
}

SbBool
CvrLightingElement::useLighting(SoState * state)
{
  const CvrLightingElement * element = (const CvrLightingElement *)
    CvrLightingElement::getConstElement(state, CvrLightingElement::classStackIndex);
  return element->lighting;
}

void
CvrLightingElement::get(SoState * state, SbVec3f & lightDirection, float & lightIntensity)
{
  const CvrLightingElement * element = (const CvrLightingElement *)
    CvrLightingElement::getConstElement(state, CvrLightingElement::classStackIndex);
  lightDirection = element->lightDirection;
  lightIntensity = element->lightIntensity;
}

// *************************************************************************

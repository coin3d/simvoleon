/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

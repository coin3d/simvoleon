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

#include <VolumeViz/elements/CvrPageSizeElement.h>

#include <assert.h>

// *************************************************************************

SO_ELEMENT_SOURCE(CvrPageSizeElement);

// *************************************************************************

void
CvrPageSizeElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(CvrPageSizeElement, inherited);
}


CvrPageSizeElement::~CvrPageSizeElement(void)
{
}

void
CvrPageSizeElement::init(SoState * state)
{
  inherited::init(state);
  this->pagesize.setValue(0, 0, 0);
}

const CvrPageSizeElement *
CvrPageSizeElement::getInstance(SoState * const state)
{
  return (const CvrPageSizeElement *)
    CvrPageSizeElement::getConstElement(state,
                                        CvrPageSizeElement::classStackIndex);
}

// *************************************************************************

SbBool
CvrPageSizeElement::matches(const SoElement * element) const
{
  return
    inherited::matches(element) &&
    ((CvrPageSizeElement *)element)->pagesize == this->pagesize;
}

SoElement *
CvrPageSizeElement::copyMatchInfo(void) const
{
  assert(this->getTypeId().canCreateInstance());

  CvrPageSizeElement * element = (CvrPageSizeElement *)
    this->getTypeId().createInstance();

  *element = *this;

  return element;
}


void
CvrPageSizeElement::set(SoState * state, const SbVec3s & value)
{
  CvrPageSizeElement * element = (CvrPageSizeElement *)
    CvrPageSizeElement::getElement(state, CvrPageSizeElement::classStackIndex);
  element->pagesize = value;
}

SbVec3s
CvrPageSizeElement::get(SoState * state)
{
  const CvrPageSizeElement * element = (const CvrPageSizeElement *)
    CvrPageSizeElement::getConstElement(state, CvrPageSizeElement::classStackIndex);
  return element->pagesize;
}

// *************************************************************************

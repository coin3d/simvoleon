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

/*!
  \class SoTransferFunctionElement VolumeViz/elements/SoTransferFunctionElement.h
  \brief This class stores a reference to the current transfer function.
*/

// *************************************************************************

#include <VolumeViz/elements/SoTransferFunctionElement.h>

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

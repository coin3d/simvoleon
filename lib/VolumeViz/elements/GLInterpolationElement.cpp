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

#include <VolumeViz/elements/CvrGLInterpolationElement.h>

#include <assert.h>

// *************************************************************************

SO_ELEMENT_SOURCE(CvrGLInterpolationElement);

// *************************************************************************

void
CvrGLInterpolationElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(CvrGLInterpolationElement, inherited);
}


CvrGLInterpolationElement::~CvrGLInterpolationElement(void)
{
}

void
CvrGLInterpolationElement::init(SoState * state)
{
  inherited::init(state);
  this->data = GL_NEAREST; // default interpolation nearest
}

const CvrGLInterpolationElement *
CvrGLInterpolationElement::getInstance(SoState * const state)
{
  return (const CvrGLInterpolationElement *)
    CvrGLInterpolationElement::getConstElement(state, CvrGLInterpolationElement::classStackIndex);
}

// *************************************************************************

void
CvrGLInterpolationElement::set(SoState * state, GLenum val)
{
  SoInt32Element::set(CvrGLInterpolationElement::classStackIndex,
                      state, NULL, val);
}

GLenum
CvrGLInterpolationElement::get(SoState * state)
{
  return SoInt32Element::get(CvrGLInterpolationElement::classStackIndex, state);
}

// *************************************************************************

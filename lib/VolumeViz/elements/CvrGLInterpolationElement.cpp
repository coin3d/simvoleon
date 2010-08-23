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

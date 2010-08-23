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

#include <VolumeViz/elements/CvrStorageHintElement.h>

#include <assert.h>

// *************************************************************************

SO_ELEMENT_SOURCE(CvrStorageHintElement);

// *************************************************************************

void
CvrStorageHintElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(CvrStorageHintElement, inherited);
}


CvrStorageHintElement::~CvrStorageHintElement(void)
{
}

void
CvrStorageHintElement::init(SoState * state)
{
  inherited::init(state);
  this->data = TRUE; // default to use paletted textures
}

const CvrStorageHintElement *
CvrStorageHintElement::getInstance(SoState * const state)
{
  return (const CvrStorageHintElement *)
    CvrStorageHintElement::getConstElement(state,
                                           CvrStorageHintElement::classStackIndex);
}

// *************************************************************************

void
CvrStorageHintElement::set(SoState * state, int val)
{
  SoInt32Element::set(CvrStorageHintElement::classStackIndex,
                      state, NULL, val);
}

int
CvrStorageHintElement::get(SoState * state)
{
  return SoInt32Element::get(CvrStorageHintElement::classStackIndex, state);
}

// *************************************************************************

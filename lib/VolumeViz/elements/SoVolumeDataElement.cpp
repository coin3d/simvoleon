/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2001 by Systems in Motion.  All rights reserved.
 *  
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use Coin with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

/*!
  \class SoVolumeDataElement Inventor/elements/SoVolumeDataElement.h
  \brief The SoVolumeDataElement class is yet to be documented.
  \ingroup elements

  FIXME: write doc.
*/

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/nodes/SoNode.h>
#include <assert.h>

/*!
  \fn SoVolumeDataElement::numCoords

  FIXME: write doc.
*/

/*!
  \fn SoVolumeDataElement::coords3D

  FIXME: write doc.
*/

/*!
  \fn SoVolumeDataElement::coords4D

  FIXME: write doc.
*/

/*!
  \fn SoVolumeDataElement::areCoords3D

  FIXME: write doc.
*/

SO_ELEMENT_SOURCE(SoVolumeDataElement);

/*!
  This static method initializes static data for the
  SoVolumeDataElement class.
*/

void
SoVolumeDataElement::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;
  SO_ELEMENT_INIT_CLASS(SoVolumeDataElement, inherited);
}


/*!
  The destructor.
*/

SoVolumeDataElement::~SoVolumeDataElement(void)
{
}

//! FIXME: write doc.

void
SoVolumeDataElement::init(SoState * state)
{
  inherited::init(state);
  volumeData = NULL;
}

//! FIXME: write doc.

void
SoVolumeDataElement::setVolumeData(SoState * const state,
                          SoNode * const node,
                          SoVolumeData * volumeData)
{
  SoVolumeDataElement * elem =
    (SoVolumeDataElement *) SoElement::getElement(state, classStackIndex);
  if (elem) {
    elem->volumeData = volumeData;
  }// if
}


//! FIXME: write doc.

SoVolumeData *
SoVolumeDataElement::getVolumeData() const
{
  return this->volumeData;
}


//! FIXME: write doc.

const SoVolumeDataElement *
SoVolumeDataElement::getInstance(SoState * const state)
{
  return (SoVolumeDataElement *)
    (getConstElement(state, classStackIndex));
}



//! FIXME: write doc.

void
SoVolumeDataElement::print(FILE * /* file */) const
{
}

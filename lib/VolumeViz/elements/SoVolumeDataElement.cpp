/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http:// www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class SoVolumeDataElement VolumeViz/elements/SoVolumeDataElement.h
  \brief 
  \ingroup elements

  FIXME: write doc.
*/

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/nodes/SoNode.h>
#include <assert.h>


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

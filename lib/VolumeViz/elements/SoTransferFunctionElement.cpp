/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http:// www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class SoTransferFunctionElement VolumeViz/elements/SoTransferFunctionElement.h
  \brief 
  \ingroup elements

  FIXME: write doc.
*/

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <Inventor/nodes/SoNode.h>
#include <assert.h>


SO_ELEMENT_SOURCE(SoTransferFunctionElement);

/*!
  This static method initializes static data for the
  SoTransferFunctionElement class.
*/

void
SoTransferFunctionElement::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;
  SO_ELEMENT_INIT_CLASS(SoTransferFunctionElement, inherited);
}


/*!
  The destructor.
*/

SoTransferFunctionElement::~SoTransferFunctionElement(void)
{
}

//! FIXME: write doc.

void
SoTransferFunctionElement::init(SoState * state)
{
  inherited::init(state);
  transferFunction = NULL;
}

//! FIXME: write doc.

void
SoTransferFunctionElement::setTransferFunction(SoState * const state,
                          SoNode * const node,
                          SoTransferFunction * transferFunction)
{
  SoTransferFunctionElement * elem =
    (SoTransferFunctionElement *) SoElement::getElement(state, classStackIndex);
  if (elem) {
    elem->transferFunction = transferFunction;
  }// if
}


//! FIXME: write doc.

SoTransferFunction *
SoTransferFunctionElement::getTransferFunction() const
{
  return this->transferFunction;
}


//! FIXME: write doc.

const SoTransferFunctionElement *
SoTransferFunctionElement::getInstance(SoState * const state)
{
  return (SoTransferFunctionElement *)
    (getConstElement(state, classStackIndex));
}



//! FIXME: write doc.

void
SoTransferFunctionElement::print(FILE * /* file */) const
{
}

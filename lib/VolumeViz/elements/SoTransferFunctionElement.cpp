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
  this->transferFunction = NULL;
}

void
SoTransferFunctionElement::setTransferFunction(SoState * const state,
                                               SoNode * const node,
                                               SoTransferFunction * func)
{
  SoTransferFunctionElement * elem = (SoTransferFunctionElement *)
    SoElement::getElement(state, SoTransferFunctionElement::classStackIndex);

  if (elem) {
    elem->transferFunction = func;
  }
}


SoTransferFunction *
SoTransferFunctionElement::getTransferFunction(void) const
{
  return this->transferFunction;
}


const SoTransferFunctionElement *
SoTransferFunctionElement::getInstance(SoState * const state)
{
  return (const SoTransferFunctionElement *)
    SoTransferFunctionElement::getConstElement(state, SoTransferFunctionElement::classStackIndex);
}


void
SoTransferFunctionElement::print(FILE * file) const
{
  // FIXME: stub. 20021106 mortene.
}

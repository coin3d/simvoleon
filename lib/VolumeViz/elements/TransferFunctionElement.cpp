/*!
  \class SoTransferFunctionElement VolumeViz/elements/SoTransferFunctionElement.h
  \brief This class stores a reference to the current transfer function.
  \ingroup volviz
*/

#include <VolumeViz/elements/SoTransferFunctionElement.h>
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
